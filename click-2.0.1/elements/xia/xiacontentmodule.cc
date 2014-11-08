#include "xiacontentmodule.hh"
#include <click/xiacontentheader.hh>
#include <click/config.h>
#include <click/glue.hh>
#include <click/xia_cache_req.h>
CLICK_DECLS

#define CACHE_DEBUG 1

unsigned int XIAContentModule::PKTSIZE = PACKETSIZE;
XIAContentModule::XIAContentModule(XIATransport *transport)
{
    _transport = transport;
    _timer=0;
#ifdef EXTERNAL_CACHE
	char buf[256];
	sprintf(buf, "/tmp/xia-%s",
			_transport->local_hid().unparse().c_str());
	loggerfp = fopen(buf, "w+");
#endif
}

XIAContentModule::~XIAContentModule()
{
    HashTable<XID,CChunk*>::iterator it;
    HashTable<XID,Packet*>::iterator it_1;

	Packet *p = NULL;
    CChunk *chunk = NULL;
    for(it=_partialTable.begin(); it!=_partialTable.end(); it++) {
        chunk=it->second;
        delete chunk;
    }
    for(it=_oldPartial.begin(); it!=_oldPartial.end(); it++) {
        chunk=it->second;
        delete chunk;
    }
    for(it=_contentTable.begin(); it!=_contentTable.end(); it++) {
        chunk=it->second;
        delete chunk;
    }
}

Packet *XIAContentModule::makeChunkResponse(CChunk * chunk, Packet *p_in)
{
    XIAHeaderEncap encap;
    XIAHeader hdr(p_in);

    encap.set_dst_path(hdr.dst_path());
    encap.set_src_path(hdr.src_path());
    encap.set_nxt(CLICK_XIA_NXT_CID);
    encap.set_plen(chunk->GetSize());

    ContentHeaderEncap  contenth(0, 0, 0, chunk->GetSize());

    uint16_t hdrsize = encap.hdr_size()+ contenth.hlen();
    //build packet
    WritablePacket *p = Packet::make(hdrsize, chunk->GetPayload() , chunk->GetSize(), 20 );

    p=contenth.encap(p);		// add XIA header
    p=encap.encap( p, false );

    return p;
}


Packet * XIAContentModule::makeChunkPush(CChunk * chunk, Packet *p_in)
{
    XIAHeader xhdr(p_in);  // parse xia header and locate nodes and payload
    ContentHeader ch(p_in);

    const unsigned char *payload=xhdr.payload();
    int offset=ch.chunk_offset();
    int length=ch.length();
    int chunkSize=ch.chunk_length();

    uint32_t ttl=ch.ttl();


    XIAHeaderEncap encap;
    XIAHeader hdr(p_in);

    encap.set_dst_path(hdr.dst_path());
    encap.set_src_path(hdr.src_path());
    encap.set_nxt(CLICK_XIA_NXT_CID);
    encap.set_plen(chunk->GetSize());

//     ContentHeaderEncap  *contenth = ContentHeaderEncap::MakePushHeader(0,  chunk->GetSize() ); //contenth(0, 0, 0, chunk->GetSize());
    ContentHeaderEncap contenth(offset, offset, length, chunkSize,
								ContentHeader::OP_PUSH, ttl);

    uint16_t hdrsize = encap.hdr_size()+ contenth.hlen();
    //build packet
    WritablePacket *p = Packet::make(hdrsize, chunk->GetPayload() , chunk->GetSize(), 20 );

    p=contenth.encap(p);		// add XIA header
    p=encap.encap( p, false );

    if(CACHE_DEBUG)
		click_chatter("Push message built in contentmodule \n");

    return p;
}

#ifdef EXTERNAL_CACHE
void XIAContentModule::store_in_external_cache(CChunk *chunk)
{
	int bytes_to_send, bytes_sent;
	xcache_req_t req;
	WritablePacket *pkt;
	char buf[MAX_TRANSFER_SIZE];

#define MIN(__a, __b) (((__a) < (__b)) ? (__a) : (__b))

	bytes_sent = 0;
	do {
		bytes_to_send = MIN((chunk->GetSize() - bytes_sent),
							MAX_TRANSFER_SIZE - sizeof(xcache_req_t));

		req.request = XCACHE_STORE;
		req.len = bytes_to_send;
		req.offset = bytes_sent;
		req.total_len = chunk->GetSize();

		req.hid = _transport->local_hid().xid();
		req.ch.cid = chunk->id().xid();
		req.ch.ttl = 86400;

		memcpy(buf, &req, sizeof(xcache_req_t));
		memcpy(buf + sizeof(xcache_req_t), chunk->GetPayload() + bytes_sent,
			   bytes_to_send);
		pkt = Packet::make(0, buf, bytes_to_send + sizeof(xcache_req_t), 0);

		_transport->checked_output_push(2 , pkt);

		bytes_sent += bytes_to_send;
	} while(bytes_sent < chunk->GetSize());
}

void XIAContentModule::search_external_cache(const XID &CID)
{
	xcache_req_t req;
	WritablePacket *pkt;

	click_chatter("Sending search request to external cache\n");
	req.request = XCACHE_SEARCH;
	req.len = 0;
	req.offset = 0;
	req.total_len = 0;

	req.hid = _transport->local_hid().xid();
	req.ch.cid = CID.xid();

	pkt = Packet::make(0, &req , sizeof(xcache_req_t), 0);
	_transport->checked_output_push(2 , pkt);
}
#endif

void XIAContentModule::process_request(Packet *p, const XID & srcHID,
									   const XID & dstCID, char *pl,
									   unsigned int s)
{
    click_chatter("process_request - local HID: %s, src HID: %s, Dest CID: %s\n"
				  , _transport->local_hid().unparse().c_str(),
				  srcHID.unparse().c_str(), dstCID.unparse().c_str());

    HashTable<XID,CChunk*>::iterator it;

#ifdef EXTERNAL_CACHE
    fprintf(loggerfp, "%s: [Lookup] local HID: %s, src HID: %s,  Dest CID: %s\n",
			__func__, _transport->local_hid().unparse().c_str(),
			srcHID.unparse().c_str(), dstCID.unparse().c_str());
	fflush(loggerfp);
	/* forward_to_external_cache(dstCID); */
#endif

    it = _contentTable.find(dstCID);

	if(malicious) {
		/* simple.html vs. simple_malicious_explanation.html */
		if (malicious &&
			strcmp(dstCID.unparse().c_str(),
				   "CID:f85579621d88b11490773d2b6196230bd2beb7b5") == 0)
		{
			/*
			 * If this router is malicous, then this content
			 * module reponds to requests for the simple.html page
			 * with a version explaining CID security
			 */
			XID fakeCID = XID();
			fakeCID.parse("CID:4e3f8b2d710eb4b1937d22bc2966462b068144d8");

			it=_contentTable.find(fakeCID);
		}

		/* image.jpg vs. anon.jpg */
		if (malicious &&
			strcmp(dstCID.unparse().c_str(),
				   "CID:8b35bac835526705709b4a9560e15a9d066a6900") == 0)
		{
			/*
			 * If this router is malicous, then this content
			 * module reponds to requests for the umbrella image
			 * with the anonymous image instead.
			 */
			XID fakeCID = XID();
			fakeCID.parse("CID:5830f441b0895cd2a82d5a13fc64f6e9a5f710ad");

			it = _contentTable.find(fakeCID);
		}

		/* 1st chunk of dongsu demo image vs. 1st chunk of anon.jpg */
		if (malicious &&
			strcmp(dstCID.unparse().c_str(),
				   "CID:fc65c7a4afaae5e59fd79af6c3f49fddbc2698de") == 0)
		{
			/*
			 * If this router is malicous, then this content
			 * module reponds to requests for the umbrella image
			 * with the anonymous image instead.
			 */
			XID fakeCID = XID();
			fakeCID.parse("CID:dd44c65bc81365ea7dfd2958732a6df937604613");

			it=_contentTable.find(fakeCID);
		}
	}

#ifdef CLIENT_CACHE_not_anymore
    if(srcHID == _transport->local_hid()) {
        ContentHeader ch(p);
        if(it!=_contentTable.end() && (content[dstCID]=1)
		   /* This is an intended assignemnt */
		   && (ch.opcode()==ContentHeader::OP_REQUEST)) {
			/* Filter out redundant request for RPT reliability */
            XIAHeaderEncap encap;
            XIAHeader hdr(p);

            XIAPath srcPath, dstPath;
            char* pl=it->second->GetPayload();
            unsigned int s=it->second->GetSize();

            handle_t s_dummy=srcPath.add_node(XID());
            handle_t s_cid=srcPath.add_node(dstCID);
            srcPath.add_edge(s_dummy, s_cid);

            handle_t d_dummy=dstPath.add_node(XID());
            XID	_destination_xid = hdr.src_path().xid(hdr.src_path().destination_node());   //Find the last node of the source, it can be a HID or SID
            handle_t d_xid=dstPath.add_node(_destination_xid);
            dstPath.add_edge(d_dummy, d_xid);

            encap.set_src_path(srcPath);
            encap.set_dst_path(dstPath);
            encap.set_plen(s);
            encap.set_nxt(CLICK_XIA_NXT_CID);

            ContentHeaderEncap  contenth(0, 0, 0, s);

            uint16_t hdrsize = encap.hdr_size()+ contenth.hlen();
            WritablePacket *newp = Packet::make(hdrsize, pl , s, 20 );

            newp=contenth.encap(newp);
            newp=encap.encap( newp, false );	      // add XIA header

 	    click_chatter("Found in my local cache! CID: %s, Local Address: %s\n", dstCID.unparse().c_str(),  _transport->local_hid().unparse().c_str());
            _transport->checked_output_push(1 , newp);
            //std::cout<<"In client"<<std::endl;
            //std::cout<<"payload: "<<pl<<std::endl;
            //std::cout<<"have pushed out"<<std::endl;
        }
        p->kill();
        return ;
    }
#endif
    // server, router
#ifdef EXTERNAL_CACHE
	if(pl && s > 0)
#else
    if(it!=_contentTable.end())
#endif
	{
        std::cout<<"look up cache in router or server"<<std::endl;
        XIAHeaderEncap encap;
        XIAHeader hdr(p);
        XIAPath myown_source;  // AD:HID:CID add_node, add_edge

        myown_source = _transport->local_addr();
        handle_t _cid=myown_source.add_node(dstCID);

        myown_source.add_edge(myown_source.source_node(), _cid);
        myown_source.add_edge(myown_source.destination_node(), _cid);
        myown_source.set_destination_node(_cid);
        encap.set_src_path(myown_source);
        encap.set_dst_path(hdr.src_path());
        encap.set_nxt(CLICK_XIA_NXT_CID);

        // add content header   dataoffset
        unsigned int cp=0;
        ContentHeaderEncap  dummy_contenth(0, 0, 0, 0);

        while(cp < s) {
            uint16_t hdrsize = encap.hdr_size()+ dummy_contenth.hlen();
            int l= (s-cp) < (PKTSIZE - hdrsize) ? (s-cp) : (PKTSIZE - hdrsize);
            ContentHeaderEncap  contenth(0, cp, l, s);
            //build packet
            WritablePacket *newp = Packet::make(hdrsize, pl + cp , l, 20 );
            newp=contenth.encap(newp);
            encap.set_plen(l);	// add XIA header
            newp=encap.encap( newp, false );
			click_chatter("Found in router cache! CID: %s, Local Address: %s\n", dstCID.unparse().c_str(),  _transport->local_hid().unparse().c_str());

			if(srcHID == _transport->local_hid())
				_transport->checked_output_push(1 , newp);
			else
				_transport->checked_output_push(0 , newp);
            std::cout<<"have pushed out"<<std::endl;
            cp += l;
        }
        p->kill();
    } else { //printf("dstID is not found in cache, pkt killed\n");
        std::cout<<"not found, kill pkt"<<std::endl;
		click_chatter("no content found\n");
        p->kill();
    }
}

void XIAContentModule::cache_incoming_forward(Packet *p, const XID& srcCID)
{
    XIAHeader xhdr(p);  // parse xia header and locate nodes and payload
    ContentHeader ch(p);

    const unsigned char *payload=xhdr.payload();
    int offset=ch.chunk_offset();
    int length=ch.length();
    int chunkSize=ch.chunk_length();

    //std::cout<<"dst is not myself"<<std::endl;
    HashTable<XID,CChunk*>::iterator it;
#ifdef EXTERNAL_CACHE
    fprintf(loggerfp, "%s: [Lookup] local HID: %s, src HID: %s\n",
			__func__, _transport->local_hid().unparse().c_str(),
			srcCID.unparse().c_str());
	fflush(loggerfp);
//	forward_to_external_cache(srcCID);
#endif
    it=_contentTable.find(srcCID);

    if (it!=_contentTable.end()) {  //already in contentTable
        content[it->first]=1;
    } else {
        it=_partialTable.find(srcCID);
        if(it!=_partialTable.end()) { //found in partialTable
            partial[it->first]=1;
            CChunk *chunk=it->second;
            chunk->fill(payload, offset, length);
            if(chunk->full()) {
#ifdef EXTERNAL_CACHE
				fprintf(loggerfp, "%s:%d: [Store] local HID: %s, src HID: %s\n",
						__func__, __LINE__,
						_transport->local_hid().unparse().c_str(),
						srcCID.unparse().c_str());
				fflush(loggerfp);
				store_in_external_cache(chunk);
#endif
                _contentTable[srcCID]=chunk;
                content[srcCID]=1;
                addRoute(srcCID);
                partial.erase(it->first);
                _partialTable.erase(it);
            }
        } else {                     //first pkt of a chunk
            CChunk *chunk=new CChunk(srcCID, chunkSize);
            chunk->fill(payload, offset, length);//  allocate space for new chunk
            MakeSpace(chunkSize);  //lru

            if(chunk->full()) {
#ifdef EXTERNAL_CACHE
				fprintf(loggerfp, "%s:%d: [Store] local HID: %s, src HID: %s\n",
						__func__, __LINE__,
						_transport->local_hid().unparse().c_str(),
						srcCID.unparse().c_str());
				fflush(loggerfp);
				store_in_external_cache(chunk);
#endif
                _contentTable[srcCID]=chunk;
                content[srcCID]=1;
                //modify routing table	  //add
                addRoute(srcCID);
            } else {
                _partialTable[srcCID]=chunk;
                partial[srcCID]=1;
            }
            usedSize += chunkSize;
        }
    }
    //lru refresh
    //printf("lru refresh\n");
    //std::cout<<timer<<std::endl;
    _timer++;
    if(_timer>=REFRESH) {
        HashTable<XID,int>::iterator iter;
        for(iter=content.begin(); iter!=content.end(); iter++) {
            iter->second=0;
        }
        for(iter=partial.begin(); iter!=partial.end(); iter++) {
            iter->second=0;
        }
        _timer=0;
    }
    p->kill();
    //printf("end: dstHID is not myself\n");

}

void XIAContentModule::cache_incoming_local(Packet* p, const XID& srcCID, bool local_putcid, bool pushcid)
{
	/* parse xia header and locate nodes and payload */
    XIAHeader xhdr(p);
    ContentHeader ch(p);
    const unsigned char *payload=xhdr.payload();
    int offset=ch.chunk_offset();
    int length=ch.length();
    int chunkSize=ch.chunk_length();
    uint32_t ttl=ch.ttl();
    HashTable<XID,CChunk*>::iterator it,oit;
    bool chunkFull=false;
    CChunk* chunk;

#ifdef EXTERNAL_CACHE
	fprintf(loggerfp, "%s:%d: [LookUP] local HID: %s, src HID: %s\n",
			__func__, __LINE__,
			_transport->local_hid().unparse().c_str(), srcCID.unparse().c_str());
	fflush(loggerfp);
#endif

#ifdef CLIENT_CACHE_not_anymore
	struct cacheMeta *cacheEntry=_cacheMetaTable.get(contextID);

    if(cacheEntry==NULL) {
        struct cacheMeta *cm=(struct cacheMeta *)malloc(sizeof(cacheMeta));
        cm->curSize=0;
        cm->maxSize=cacheSize;
        cm->policy=cachePolicy;
        cm->contentMetaTable=new HashTable<XID, struct contentMeta*>();
        _cacheMetaTable[contextID]=cm;
        if(CACHE_DEBUG){
            click_chatter("Create new cacheMeta struct for id[%d]\n", contextID);
        }
    }
	HashTable<XID,CChunk*>::iterator cit;

	_timer++;
	cit=_contentTable.find(srcCID);
	if(cit!=_contentTable.end()) {
		/* content exists alreaady */
		if (pushcid) {
			/* click_chatter("Pushing something that we already have\n"); */
			chunk=cit->second;
			chunk->fill(payload, offset, length);
			if(chunk->full()) {
				/* chunkFull=true; */
				Packet *newp = makeChunkResponse(chunk, p);
				_transport->checked_output_push(1 , newp);
			}else
				click_chatter("Why is a partial chunk in contentTable?\n");

			if(_timer>=REFRESH) {
                _timer = 0;
                cache_management();
			}
	    } else {
			if (!local_putcid)
				content[srcCID]=1;
			p->kill();

			if(_timer>=REFRESH) {
				_timer = 0;
				cache_management();
			}
			return;
	    }
	}
#endif
    it=_partialTable.find(srcCID);
	if(it!=_partialTable.end()) {
		/* already in partial table */
        chunk=it->second;
        chunk->fill(payload, offset, length);
        if(chunk->full()) {
            chunkFull=true;
            _partialTable.erase(it);
        }
    } else {
        oit=_oldPartial.find(srcCID);
		if(oit!=_oldPartial.end()) {
			/* already in old partial table */
            chunk=oit->second;
            chunk->fill(payload, offset, length);
            _oldPartial.erase(oit);
            if(chunk->full()) {
                chunkFull=true;
            } else {
                _partialTable[srcCID]=chunk;
            }
		} else {
			/* first pkt to the client */
            chunk=new CChunk(srcCID, chunkSize);
            chunk->fill(payload, offset, length);
            if(chunk->full()){
                chunkFull=true;
            }else{
                _partialTable[srcCID]=chunk;
            }
        }
    }

    if(chunkFull) {
		/* have built the whole chunk pkt */
        if (!local_putcid && !pushcid) {
			/* sendout response to upper layer (application) */
            Packet *newp = makeChunkResponse(chunk, p);
            _transport->checked_output_push(1 , newp);
        }
		if (pushcid) {
			/* send push to upper layer (application) */
            Packet *newp = makeChunkPush(chunk, p);
            _transport->checked_output_push(1 , newp);
		}
		addRoute(srcCID);
#ifdef EXTERNAL_CACHE
		fprintf(loggerfp, "%s:%d: [Store] local HID: %s, src HID: %s\n",
				__func__, __LINE__,
				_transport->local_hid().unparse().c_str(),
				srcCID.unparse().c_str());
		fflush(loggerfp);
		store_in_external_cache(chunk);
#endif

#ifdef CLIENT_CACHE_not_anymore
        if (local_putcid || _cache_content_from_network) {
            struct cacheMeta *cm = _cacheMetaTable[contextID];
			struct contentMeta *ctm =
				(struct contentMeta *)malloc(sizeof(contentMeta));
            HashTable <XID, struct contentMeta*> *cmTable =
				cm->contentMetaTable;

			cm->curSize += chunkSize;
            ctm->chunkSize = chunkSize;
            ctm->ttl = ttl;
            gettimeofday(&(ctm->timestamp), NULL);

            (*cmTable)[srcCID]=ctm;

            _contentTable[srcCID]=chunk;
            if (local_putcid) {
                assert(ContentHeader::OP_LOCAL_PUTCID>1);
                content[srcCID]= ContentHeader::OP_LOCAL_PUTCID;
            } else {
                content[srcCID]= 1;
            }

            addRoute(srcCID);
            applyLocalCachePolicy(contextID);
        } else {
            if (CACHE_DEBUG)
                click_chatter("LOCAL %s delete CID %s",_transport->local_hid().unparse().c_str(), srcCID.unparse().c_str());
            delete chunk;
        }
#else
        delete chunk;
#endif
    }
    if(_timer >= REFRESH) {
        _timer = 0;
        //trigger cache_management();
        cache_management();
    }

    p->kill();
}

/**
 * @brief Clean up local cache based on policy
 *
 * @returns Void
 */ 
void XIAContentModule::applyLocalCachePolicy(int contextID){
#ifdef CLIENT_CACHE_not_anymore
    struct cacheMeta *cm=_cacheMetaTable[contextID];
    switch(cm->policy){
    case POLICY_FIFO:
		if(CACHE_DEBUG){
			click_chatter("Cache Size %d/%d\n", cm->curSize, cm->maxSize);
		}
		if(cm->maxSize!=0 && cm->curSize>cm->maxSize){
			bool done=false;
			HashTable<XID, struct contentMeta*> *cmTable=cm->contentMetaTable;
			/* Remove expired content until curSize < maxSize */
			struct contentMeta *cPtr;
			int chunkSize;
			HashTable<XID,CChunk*>::iterator cit;
			cit=_contentTable.begin();
			while(cit!=_contentTable.end()) {
				int contentType=content[cit->first];
				if(contentType == ContentHeader::OP_LOCAL_PUTCID){
					cPtr=(*cmTable)[cit->first];
					if(isExpiredContent(cPtr)){
						chunkSize=cPtr->chunkSize;
						cm->curSize-=chunkSize;
						cmTable->erase(cit->first);
						content.erase(cit->first);
						if(CACHE_DEBUG){
							click_chatter("RM [%s] Size: %d\n", cit->first.unparse().c_str(), 
										  chunkSize);
						}
						_contentTable.erase(cit);
						free(cPtr);
						if(cm->curSize < cm->maxSize){
							done=true;
							break;
						}
					}
				}
				cit++;
			}
			/* Remove old but active contents to make space for new one */
			if(!done){
				while(cm->curSize>cm->maxSize){
					const XID *minID=findOldestContent(contextID);
					if(minID!=NULL){
						const XID tmp=*minID;
						cPtr=(*cmTable)[tmp];
						chunkSize=cPtr->chunkSize;
						cm->curSize-=chunkSize;
						cmTable->erase(tmp);
						content.erase(tmp);
						if(CACHE_DEBUG){
							click_chatter("RM [%s] Size: %d\n", _contentTable.find(tmp)->first.unparse().c_str(), 
										  chunkSize);
						}
						_contentTable.erase(tmp);
						free(cPtr);
					}
				}
			}
		}
		break;
	default:
		break;
    }
#endif
}

/**
 * @brief check if a content is expired
 * 
 * @returns True if it is expired
 */
bool XIAContentModule::isExpiredContent(struct contentMeta* cm){
    int ttl=cm->ttl;
    struct timeval *thisTime=&(cm->timestamp);    
    struct timeval curTime;
    gettimeofday(&curTime,NULL);
    if(thisTime->tv_sec+ttl >= curTime.tv_sec){
        return true;
    }else{
        return false;
    }
}

const XID* XIAContentModule::findOldestContent(int contextID){
    struct cacheMeta *cm=_cacheMetaTable[contextID];
    HashTable<XID,CChunk*>::iterator cit;
    cit=_contentTable.begin();
    const XID *minID=NULL;
    struct timeval minTime;
    gettimeofday(&minTime, NULL);
    HashTable<XID, struct contentMeta*>* cmTable=cm->contentMetaTable;
    struct contentMeta *cPtr;
    while(cit!=_contentTable.end()) {
        int contentType=content[cit->first];
        if(contentType == ContentHeader::OP_LOCAL_PUTCID){
            cPtr=(*cmTable)[cit->first];
            if(cPtr->timestamp.tv_sec<=minTime.tv_sec){
                minTime=cPtr->timestamp;
                minID=&(cit->first);
            }
        }
        cit++;
    }
    return minID;
}

void XIAContentModule::cache_incoming_remove(Packet *p, const XID& srcCID) {
#ifdef CLIENT_CACHE_not_anymore

    XIAHeader xhdr(p); 
    ContentHeader ch(p);

    struct cacheMeta *cm=_cacheMetaTable[contextID];
    if(cm!=NULL) {
        HashTable<XID, struct contentMeta*> *cmTable=cm->contentMetaTable;
        struct contentMeta* cPtr=(*cmTable)[srcCID];
        if(cPtr!=NULL){
            int chunkSize=cPtr->chunkSize;
            cm->curSize-=chunkSize;
            cmTable->erase(srcCID);
            content.erase(srcCID);
            if(CACHE_DEBUG){
				click_chatter("RMCID Request [%s] Size: %d\n", _contentTable.find(srcCID)->first.unparse().c_str(), 
							  chunkSize);
            }
            delRoute(srcCID);
#ifdef EXTERNAL_CACHE
			fprintf(loggerfp, "%s:%d: [Remove] local HID: %s, src HID: %s\n",
					__func__, __LINE__,
					_transport->local_hid().unparse().c_str(),
					srcCID.unparse().c_str());
			fflush(loggerfp);
//			forward_to_external_cache(srcCID);
#endif
            _contentTable.erase(srcCID);
            free(cPtr);
            if(CACHE_DEBUG){
				click_chatter("Cache Size %d/%d\n", cm->curSize, cm->maxSize);
            }
        }
    }

#endif
}

void XIAContentModule::cache_management()
{
    HashTable<XID,CChunk*>::iterator cit;
    HashTable<XID,CChunk*>::iterator it;
    CChunk *chunk = NULL;

    for(it=_oldPartial.begin(); it!=_oldPartial.end(); it++) {
        chunk=it->second;
        if (CACHE_DEBUG)
            click_chatter("oldPartial %s delete CID %s",_transport->local_hid().unparse().c_str(), chunk->id().unparse().c_str());
        delete chunk;
    }
    _oldPartial.clear();
    _oldPartial=_partialTable;
    /* Never delete chunk in the _partialTable here
       because the chunks are still used in the _oldPartialTable
       This have created a bug before.  */
    _partialTable.clear();
#ifdef CLIENT_CACHE_not_anymore
    cit=_contentTable.begin();
    while(cit!=_contentTable.end()) {
        int contentType=content[cit->first];
        if( contentType == 0 ) {
            HashTable<XID, CChunk*>::iterator pit=cit;
            _oldPartial[pit->first]=pit->second;
            delRoute(pit->first);
            content.erase(pit->first);
#ifdef EXTERNAL_CACHE
			fprintf(loggerfp, "%s:%d: [Remove] local HID: %s\n",
					__func__, __LINE__,
					_transport->local_hid().unparse().c_str());
			fflush(loggerfp);
#endif
            _contentTable.erase(pit);
        }else if(contentType != ContentHeader::OP_LOCAL_PUTCID){
            content[cit->first]=0;
        }
        cit++;
    }
#endif
}

/* source ID is the content */
void
XIAContentModule::cache_incoming(Packet *p, const XID& srcCID,
								 const XID& dstHID, int /*port*/)
{
    XIAHeader xhdr(p);  // parse xia header and locate nodes and payload
    ContentHeader ch(p);
    bool local_putcid = (ch.opcode() == ContentHeader::OP_LOCAL_PUTCID);
    bool local_removecid= (ch.opcode() == ContentHeader::OP_LOCAL_REMOVECID);
    bool local_pushcid= (ch.opcode() == ContentHeader::OP_PUSH);

    if (CACHE_DEBUG) {
        click_chatter("--Cache incoming--%s %s",
					  srcCID.unparse().c_str(),
					  _transport->local_hid().unparse().c_str());
	}
	/**
	 * FIXME: This comparison is just wrong. dstHID that is passed is hardcoded
	 * and sometimes different types are being compared
	 */
    if(local_putcid || dstHID == _transport->local_hid()) {
        /*
		 * cache in client: if it is local putCID() then store content.
		 * Otherwise, should return the whole chunk if possible
		 * printf("cache_incoming_local - local HID: %s, Dest HID: %s\n",
		 * _transport->local_hid().unparse().c_str(),
		 * dstHID.unparse().c_str());
		 */
		click_chatter("Calling cache_incoming_local\n");
        cache_incoming_local(p, srcCID, local_putcid, local_pushcid);
		click_chatter("Finished cache_incoming_local\n");
    } else if(local_removecid) {
		/*
		 * printf("cache_incoming_remove - local HID: %s, Dest HID: %s\n",
		 * _transport->local_hid().unparse().c_str(), dstHID.unparse().c_str());
		 */
        cache_incoming_remove(p, srcCID);
    }
    else{
		/* cache in server, router */
		/* printf("cache_incoming_forward - local HID: %s, Dest HID: %s\n",
		 * _transport->local_hid().unparse().c_str(), dstHID.unparse().c_str());
		 */
		click_chatter("Calling cache_incoming_forward\n");
        cache_incoming_forward(p, srcCID);
	}
}

int
XIAContentModule::MakeSpace(int chunkSize)
{
    while( usedSize + chunkSize > MAXSIZE) {
        HashTable<XID,int>::iterator i;
        for(i=partial.begin(); i!=partial.end(); i++)
            if(i->second==0)
                break;


        if(i!=partial.end()) {
            HashTable<XID,CChunk*>::iterator iter = _partialTable.find(i->first);
            usedSize-=iter->second->GetSize();

            CChunk *t=iter->second;

            _partialTable.erase(iter);
            partial.erase(i);

            delete t;
            continue;
        }

        for(i=content.begin(); i!=content.end(); i++) {
            if(i->second==0)
                break;
        }

        if(i!=content.end()) {
            HashTable<XID,CChunk*>::iterator iter=_contentTable.find(i->first);
            usedSize-=iter->second->GetSize();
            // modify the routin table	     //delete
            delRoute(iter->first);
            CChunk *t=iter->second;
#ifdef EXTERNAL_CACHE
			fprintf(loggerfp, "%s:%d: [Remove] local HID: %s\n",
					__func__, __LINE__,
					_transport->local_hid().unparse().c_str());
			fflush(loggerfp);
#endif
            _contentTable.erase(iter);
            content.erase(i);
            delete t;
            continue;
        }

        HashTable<XID,CChunk*>::iterator iter=_partialTable.begin();
        if(iter!=_partialTable.end()) {
            usedSize-=iter->second->GetSize();
            CChunk *t=iter->second;
            _partialTable.erase(iter);
            partial.erase(i);
            delete t;
            continue;
        }

        iter = _contentTable.begin();
        usedSize-=iter->second->GetSize();
        //modify routing table	   //delete

        delRoute(iter->first);
        CChunk *t=iter->second;
#ifdef EXTERNAL_CACHE
		fprintf(loggerfp, "%s:%d: [Remove] local HID: %s\n",
				__func__, __LINE__,
				_transport->local_hid().unparse().c_str());
		fflush(loggerfp);
#endif
        _contentTable.erase(iter);
        content.erase(iter->first);
        delete t;
    }
    return 0;
}

CPart::CPart(unsigned int off, unsigned int len)
{
    offset=off;
    length=len;
}

CChunk::CChunk(XID _xid, int chunkSize): deleted(false)
{
    size=chunkSize;
    complete=false;
    payload=new char[size];
    xid=_xid;
}

CChunk::~CChunk()
{
    /* TODO: Memory leak prevention -- CPartList has to be deallocated */
    delete payload;
}

void CChunk::Merge(CPartList::iterator it)
{
    //std::cout<<"enter Merge: offset is "<< it->offset <<std::endl;
    CPartList::iterator post_it;
    post_it=it;
    post_it++;

    while(1) {
        if(post_it==parts.end()) break;
        if(post_it->v.offset > it->v.offset+it->v.length) {
            break;
        } else {
            unsigned int l=it->v.offset+it->v.length;
            unsigned int _l=post_it->v.offset+post_it->v.length;
            if(l>_l) {
                CPartListNode *n = post_it.get();
                parts.erase(post_it);
                delete n;
            } else {
                it->v.length= _l - it->v.offset;
                CPartListNode *n = post_it.get();
                parts.erase(post_it);
                delete n;
            }
            post_it=it;
            post_it++;
        }
    }
}

int
CChunk::fill(const unsigned char *_payload, unsigned int offset, unsigned int length)
{
    //  std::cout<<"enter fill, payload is "<<_payload<<std::endl;

    CPartList::iterator it, post_it;
    CPart p(offset, length);
    char *off = payload + offset ;

    for( it=parts.begin(); it!=parts.end(); it++ ) {
        if(it->v.offset > offset)break;
    }
    if(it==parts.end()) {  //end
        if(it==parts.begin()) { //empty
            //std::cout<<"push back"<<std::endl;
            memcpy(off, _payload, length);
            parts.push_back(new CPartListNode(p));
        } else {            //not empty
            it--;
            if( it->v.offset + it->v.length < offset + length) {
                memcpy(off, _payload, length);
                //std::cout<<"off is "<<off<<std::endl;
                //std::cout<<"push back"<<std::endl;
                parts.push_back(new CPartListNode(p));
                Merge(it);
            }
        }
    } else { //not end
        if(it==parts.begin()) { //begin
            memcpy(off, _payload, length);
            //std::cout<<"push front"<<std::endl;
            parts.push_front(new CPartListNode(p));
            it=parts.begin();
            Merge(it);
        } else {	//not begin
            it--;
            if(offset <= it->v.offset + it->v.length ) { //overlap with previous interval
                if( offset+length <= it->v.offset + it->v.length ) { //cover
                } else { //not cover
                    memcpy(off, _payload, length);
                    it->v.length = (offset+length) - it->v.offset;
                    Merge(it);
                }
            } else { // not overlap with previous interval
                memcpy(off,_payload, length);
                //std::cout<<"insert"<<std::endl;
                parts.insert(post_it, new CPartListNode(p));
                post_it=it;
                post_it++;
                Merge(post_it);
            }
        }
    }
    //  std::cout<<"leave fill: list size is "<<parts.size()<<std::endl;
    return 0;
}


bool
CChunk::full()
{
    if(complete==true) return true;

    CPartList::iterator it;
    it= parts.begin();

    if( it->v.offset==0 && it->v.length==size) {
        complete=true;
        return true;
    }
    return false;
}

CLICK_ENDDECLS
//ELEMENT_REQUIRES(userlevel)
ELEMENT_PROVIDES(XIAContentModule)

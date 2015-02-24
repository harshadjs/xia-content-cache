STORE_AUTOGEN_HDR="${XCACHEDIR}/bin/_stores.autogen.h"

stores: create_hdr
##
## After creating header file, invoke "make" for each individual
## store.
##
	@while read -r line; do \
	 	PRI=`expr $$PRI + 1`; \
		store=`echo $$line | cut -d "#" -f1`; \
		echo "store = $$store"; \
		if [ "$$store" = "" ]; then \
			continue; \
		fi; \
	 	make -C $$line CFLAGS="$$CFLAGS -DXCACHE_STORE_PRIO=$$PRI -D_XCACHE_STORE_INIT=_$${store}_init -I$${XCACHEDIR}/bin"; \
	 	mv -f $$store/*.o .; \
	 done < order.conf

##
## Create header file _stores.autogen.h
##
create_hdr:
	@echo -e "/* This is an automatically generated file. */\n" \
		> $(STORE_AUTOGEN_HDR)
	@echo "enum {" >> $(STORE_AUTOGEN_HDR)
	@echo "" >> $(STORE_AUTOGEN_HDR)
	@PRI=0; \
	while read -r line; do \
		store=`echo $$line | cut -d "#" -f1`; \
		echo "hdr_store = $$store"; \
		if [ "$$store" = "" ]; then \
			continue; \
		fi; \
		PRI=`expr $$PRI + 1`; \
		echo -e "\t xcache_store_$${store}," >> $(STORE_AUTOGEN_HDR); \
	done < order.conf; \
	echo "};" >> $(STORE_AUTOGEN_HDR); \
	echo "" >> $(STORE_AUTOGEN_HDR); \
	echo "#define XCACHE_N_STORES $${PRI}" >> $(STORE_AUTOGEN_HDR)

clean:
	rm -f $(STORE_AUTOGEN_HDR)

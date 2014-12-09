PLUGIN_AUTOGEN_HDR="_plugins.autogen.h"

plugins: create_hdr
	@while read -r plugin; do \
	 	PRI=`expr $$PRI + 1`; \
	 	make -C $$plugin CFLAGS="$$CFLAGS -DXCACHE_PLUGIN_PRIO=$$PRI -D_XCACHE_PLUGIN_INIT=_$${plugin}_init -I$${XCACHEDIR}/plugins"; \
	 	if [ "$$?" != "0" ]; then \
	 		exit; \
	 	fi; \
	 	mv -f $$plugin/*.o .; \
	 done < order.conf

create_hdr:
	@echo "/* This is an automatically generated file. */" \
		> $(PLUGIN_AUTOGEN_HDR)
	@echo "enum {" >> $(PLUGIN_AUTOGEN_HDR)
	@echo "" >> $(PLUGIN_AUTOGEN_HDR)
	@PRI=0; \
	while read -r plugin; do \
		PRI=`expr $$PRI + 1`; \
		echo "\t_xcache_plugin_$${plugin}," >> $(PLUGIN_AUTOGEN_HDR); \
	done < order.conf; \
	echo "};" >> $(PLUGIN_AUTOGEN_HDR); \
	echo "" >> $(PLUGIN_AUTOGEN_HDR); \
	echo "#define XCACHE_N_PLUGINS $${PRI}" >> $(PLUGIN_AUTOGEN_HDR)

clean:
	rm -f $(PLUGIN_AUTOGEN_HDR)

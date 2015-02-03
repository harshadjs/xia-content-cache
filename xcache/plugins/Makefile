PLUGIN_AUTOGEN_HDR="${XCACHEDIR}/bin/_plugins.autogen.h"

plugins: create_hdr
##
## After creating header file, invoke "make" for each individual
## plugin.
##
	@while read -r line; do \
	 	PRI=`expr $$PRI + 1`; \
		plugin=`echo $$line | cut -d "#" -f1`; \
		echo "plugin = $$plugin"; \
		if [ "$$plugin" = "" ]; then \
			continue; \
		fi; \
	 	make -C $$line CFLAGS="$$CFLAGS -DXCACHE_PLUGIN_PRIO=$$PRI -D_XCACHE_PLUGIN_INIT=_$${plugin}_init -I$${XCACHEDIR}/bin"; \
	 	mv -f $$plugin/*.o .; \
	 done < order.conf

##
## Create header file _plugins.autogen.h
##
create_hdr:
	@echo -e "/* This is an automatically generated file. */\n" \
		> $(PLUGIN_AUTOGEN_HDR)
	@echo "enum {" >> $(PLUGIN_AUTOGEN_HDR)
	@echo "" >> $(PLUGIN_AUTOGEN_HDR)
	@PRI=0; \
	while read -r line; do \
		plugin=`echo $$line | cut -d "#" -f1`; \
		echo "hdr_plugin = $$plugin"; \
		if [ "$$plugin" = "" ]; then \
			continue; \
		fi; \
		PRI=`expr $$PRI + 1`; \
		echo -e "\t xcache_plugin_$${plugin}," >> $(PLUGIN_AUTOGEN_HDR); \
	done < order.conf; \
	echo "};" >> $(PLUGIN_AUTOGEN_HDR); \
	echo "" >> $(PLUGIN_AUTOGEN_HDR); \
	echo "#define XCACHE_N_PLUGINS $${PRI}" >> $(PLUGIN_AUTOGEN_HDR)

clean:
	rm -f $(PLUGIN_AUTOGEN_HDR)

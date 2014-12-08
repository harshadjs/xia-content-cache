plugins:
	@while read -r plugin; do \
		PRI=`expr $$PRI + 1`; \
		make -C $$plugin CFLAGS="$$CFLAGS -DXCACHE_PLUGIN_PRIO=$$PRI \
				-D_XCACHE_PLUGIN_INIT=_$${plugin}_init"; \
		if [ "$$?" != "0" ]; then \
			exit; \
		fi; \
		mv -f $$plugin/*.o .; \
	done < order.conf
clean:

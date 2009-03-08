#!/usr/bin/make -f
# Makefile (installation) for blockcontrol

SHELL = /bin/sh

clean:
	rm -f *~

install:
	install -D -m 755 blockcontrol $(DESTDIR)/usr/bin/blockcontrol
	install -D -m 755 blockcontrol.main $(DESTDIR)/usr/lib/blockcontrol/blockcontrol.main
	install -D -m 644 blockcontrol.defaults $(DESTDIR)/usr/lib/blockcontrol/blockcontrol.defaults
	install -D -m 644 blockcontrol.lib $(DESTDIR)/usr/lib/blockcontrol/blockcontrol.lib
	install -D -m 644 allow.p2p $(DESTDIR)/etc/blockcontrol/allow.p2p
	install -D -m 644 blockcontrol.conf $(DESTDIR)/etc/blockcontrol/blockcontrol.conf
	install -D -m 644 blocklists.list $(DESTDIR)/etc/blockcontrol/blocklists.list
	install -D -m 755 iptables-custom-insert.sh $(DESTDIR)/etc/blockcontrol/iptables-custom-insert.sh
	install -D -m 755 iptables-custom-remove.sh $(DESTDIR)/etc/blockcontrol/iptables-custom-remove.sh
	install -D -m 755 if-up $(DESTDIR)/etc/network/if-up.d/blockcontrol
	install -D -m 755 cron.daily $(DESTDIR)/etc/cron.daily/blockcontrol
	install -D -m 755 init $(DESTDIR)/etc/init.d/blockcontrol
# 	update-rc.d defaults 20
	install -D -m 644 logrotate $(DESTDIR)/etc/logrotate.d/blockcontrol
	install -d $(DESTDIR)/var/lib/blockcontrol
	install -d $(DESTDIR)/var/spool/blockcontrol

.PHONY: clean

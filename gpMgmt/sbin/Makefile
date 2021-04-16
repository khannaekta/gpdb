# gpMgmt/sbin/Makefile

top_builddir = ../..
ifneq "$(wildcard $(top_builddir)/src/Makefile.global)" ""
include $(top_builddir)/src/Makefile.global
endif

PYCS= gpconfig_helper.pyc gpoperation.pyc gpsegstop.pyc gpcleansegmentdir.pyc \
	gpgetstatususingtransition.pyc gpsegstart.pyc

PROGRAMS= gpconfig_helper.py gpoperation.py gpsegstop.py gpcleansegmentdir.py \
	gpgetstatususingtransition.py gpsegstart.py packcore

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(sbindir)'

install: installdirs
	for file in $(PROGRAMS); do \
		pyinstaller --paths=/tmp/pydeps/:lib:../bin/gppylib --onefile $$file; \
		if [ $$? == 0 ]; then \
			$(INSTALL_SCRIPT) dist/$$file '$(DESTDIR)$(sbindir)/'$$file ; \
		else \
			$(INSTALL_SCRIPT) $$file '$(DESTDIR)$(sbindir)/'$$file ; \
		fi; \
		$(PERL) $(top_builddir)/putversion '$(DESTDIR)$(sbindir)/'$$file ; \
	done

uninstall:
	for file in $(PROGRAMS); do \
		rm -f '$(DESTDIR)$(sbindir)/'$$file ; \
	done

clean distclean:
	rm -f $(PYCS)

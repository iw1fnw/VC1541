@c:
@cd \projects\vc1541_2
@vmode 100
@mkdir d:\tmp
@set TMPDIR=d:\tmp
@set RHIDE_LIBS=$(addprefix -l,$(LIBS) $(RHIDE_TYPED_LIBS) $(RHIDE_OS_LIBS)) obj/exithook.o
@rhide vc1541_2
@set RHIDE_LIBS=

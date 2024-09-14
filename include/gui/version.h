#ifndef __gui_version_h
#define __gui_version_h

#define UI_NAME		"VC1541"
#define UI_MAJOR	0
#define UI_MINOR	4
#define UI_PATCHLEVEL	1

#if UI_PATCHLEVEL == 0
#define UI_MK_VERSION2(min,maj,pl)	"V"#maj"."#min
#define UI_MK_VERSION1(min,maj,pl)	UI_MK_VERSION2(maj,min,pl)
#else
#define UI_MK_VERSION2(min,maj,pl)	"V"#maj"."#min"pl"#pl
#define UI_MK_VERSION1(min,maj,pl)	UI_MK_VERSION2(maj,min,pl)
#endif
#define UI_VERSION	UI_MK_VERSION1(UI_MAJOR,UI_MINOR,UI_PATCHLEVEL)

#endif /* __gui_version_h */

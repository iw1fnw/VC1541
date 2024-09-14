/*
 *  $Id: cmdline.h,v 1.3 1998/10/26 03:03:24 tp Exp $
 *
 *
 *  Copyright (C) 1997 Torsten Paul
 *
 *  This file is part of VC1541, the Commodore Floppy 1541 Emulator.
 */

#ifndef __misc_cmdline_h
#define __misc_cmdline_h

typedef bool (*ConstrainFunc)(const char *, const char *);

struct _CmdArguments
{
	char *name;
	bool has_arg;
	bool is_given;
	char *arg;
	ConstrainFunc func;
	char *desc;
};
typedef struct _CmdArguments CmdArguments;

class CmdLineArgs
{
private:
	static CmdLineArgs *_self;
	static char *_program;
	static CmdArguments _arg[];

	int _lpt;
protected:
	const CmdArguments * findArg(const char *arg);
public:
	CmdLineArgs(int *argc, char ***argv);
	virtual ~CmdLineArgs(void);

	const char * programName(void);
	bool isGiven(const char *name);
	int getIntValue(const char *name);
	int getIntValue(const char *name, int def);
	const char * getStringValue(const char *name);
	const char * getStringValue(const char *name, const char *def);

	void showArgs(void);

	static const CmdLineArgs * getInstance(void);
};

#endif /* __misc_cmdline_h */


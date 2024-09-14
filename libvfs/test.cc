#include <iostream.h>

#include "libvfs/fs_fact.h"

void printdir(Device *dev)
{
	Directory *dir;
	DirectoryEntry *de;

	dir = dev->readdir();
	cout << "--------------------------------------------" << endl;
	for (Directory::iterator it = dir->begin();it != dir->end();it++) {
		cout << "\t" << (*it)->type() << " " << (*it)->name() << endl;
	}
	cout << "--------------------------------------------" << endl;
}

void mainloop(void)
{
	char buf[100];
	FSFactory fs_factory;

	Device *dev = new Device(&fs_factory, 8);

	while (1) {
		cout << "--- current path is: " << dev->path() << endl;
		printdir(dev);
		do {
			cout << "chdir: ";
			if (!fgets(buf, 100, stdin)) return;
			if (strrchr(buf, '\n')) {
				*(strrchr(buf, '\n')) = '\0';
			}
		} while (dev->chdir(buf));
	}
}

int main(void)
{
	mainloop();

	cout << endl;

	return 0;
}

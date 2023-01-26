#pragma once


struct msl_header {
	int field0;
	int files;
	int endOffset;
	int field12;
	int fileSize;
	int field20;
	int field24;
};


struct msl_file {
	int offset;
	int size;
	short id;
	short field10;
};
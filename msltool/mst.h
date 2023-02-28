#pragma once

struct mst_header {
	int header;
	int field4;
	short sounds;
	short table2Entries;
	short table3Entries;
	short field14;

	int soundsPointer;
	int table2Pointer;
	int table3Pointer;
	int table4Pointer;
	int listPointer;
	int fileSize[2];
	char pad[52];
};

struct sound_entry{
	int field0;
	int field4;
	short soundID;
	short unk;
	int frequency;
	int field16;
	int field20;
	int field24;
	int field28;
	int field32;
};


struct table2_entry {
	int field0;
	int field4;
	int field8;
	int field12;
};

struct table3_entry {
	int field0;
	int field4;
	int field8;
	int field12;
};

struct table4_entry {
	short soundID;
	short field2;
	int field4;
	int field8;
	int field12;
	int field16;
	int field20;
	int field24;
	int field28;
	int field32;
	int field36;
	int field40;
	int field44;
	int field48;
	int field52;
	int field56;
	int field60;
	int field64;
	int field68;
	int field72;
	int field76;

};

struct table4_extraentry {
	int field0;
	int field4;
	int field8;
	int field12;
	int field16;
	int field20;
	int field24;
	int field28;
	int field32;
	int field36;
};
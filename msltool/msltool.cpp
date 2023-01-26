// msltool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include "msl.h"
#include "mst.h"

#define HEADERS_FOLDER_NAME "headers"
#define SFXDATA_FOLDER_NAME "sfxdata"

#define MST_PAD_VALUE 32

int makePad(int value, int pad)
{
    return (value + pad - 1) & -pad;
}

int main(int argc, char* argv[])
{

    if (argc == 1) {
        std::cout << "Usage: msltool <optional params> <file/folder>\n"
            << "Params if a folder is used for input:\n"
            << "    -t	Generate MST file from input folder (MKA/MKD only).\n"
            << "    -l  Generate MS2 file from input folder.\n";
        return 1;
    }

    std::string input = argv[argc - 1];

    bool isFolder = std::filesystem::is_directory(input);

    if (!isFolder)
    {
        std::transform(input.begin(), input.end(), input.begin(), tolower);
        std::string extension = input.substr(input.find_last_of(".") + 1);
        std::ifstream pFile(input, std::ofstream::binary);
  
        if (pFile)
        {
            if (extension == "ms2")
            {
                msl_header msl;
                pFile.read((char*)&msl, sizeof(msl_header));

                std::cout << "Files: " << msl.files << std::endl;

                std::vector<msl_file> files;
                std::vector<std::string> banks;
                int numBank = 0;
                int numSound = 0;

                for (int i = 0; i < msl.files - 2; i++)
                {
                    msl_file file;
                    pFile.read((char*)&file, sizeof(msl_file));

                    files.push_back(file);
                }


                char pad[9];
                pFile.read(pad, sizeof(pad));

                std::string name;
                std::string dummy = "dummy";
                while (std::getline(pFile, name, '\0'))
                {
                    int pos = (int)pFile.tellg();
                    banks.push_back(name);

                    if (pos == msl.endOffset + 12)
                        break;
                }

                std::string folder = input.substr(0, input.find_last_of('.'));

                std::filesystem::create_directory(folder);
                std::filesystem::current_path(folder);

                std::filesystem::create_directory(HEADERS_FOLDER_NAME);
                std::filesystem::create_directory(SFXDATA_FOLDER_NAME);

                std::ofstream oTable(folder + ".txt");

                for (unsigned int i = 0; i < files.size(); i++)
                {

                    bool amIBankHeader = false;
                    pFile.seekg(files[i].offset, pFile.beg);
                    int data = 0;
                    pFile.read((char*)&data, sizeof(int));
                    std::string output;
                    if (data == 0xB || data == 6)
                    {
                        numBank++;
                        amIBankHeader = true;
                        output = HEADERS_FOLDER_NAME"\\";
                    }
                    else
                    {
                        numSound++;
                        amIBankHeader = false;
                        output = SFXDATA_FOLDER_NAME"\\";
                    }
                    pFile.seekg(files[i].offset, pFile.beg);

                    if (amIBankHeader)
                        output += banks[numBank - 1];
                    else
                        output += std::to_string(i + 1) + "_" + std::to_string(files[i].id) + ".sfx";

                    std::cout << "Processing: " << output << std::endl;
                    std::ofstream oFile(output, std::ofstream::binary);

                    oTable << files[i].id << " " << files[i].field10 << " " << output << std::endl;


                    int dataSize = files[i].size;
                    std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
                    pFile.read(dataBuff.get(), dataSize);
                    oFile.write(dataBuff.get(), dataSize);
                }
                oTable.close();
                pFile.close();
                std::cout << "Banks: " << numBank << " Sounds: " << numSound << std::endl;
                std::cout << "Finished." << std::endl;
                return 0;
            }
            if (extension == "mst")
            {
                mst_header mst;
                
                pFile.read((char*)&mst, sizeof(mst_header));

                if (!(mst.header == 0xB))
                {
                    std::cout <<"ERROR: " << input << " is not a valid MST file!" << std::endl;
                    return 0;
                }

                std::string folder = input.substr(0, input.find_last_of('.'));

                std::filesystem::create_directory(folder);
                std::filesystem::current_path(folder);

                std::cout << "INFO: Processing table 1 (sounds)" << std::endl;
                pFile.seekg(mst.soundsPointer, pFile.beg);

                std::vector<sound_entry> sounds;
                {
                    for (int i = 0; i < mst.sounds; i++)
                    {
                        sound_entry snd;
                        pFile.read((char*)&snd, sizeof(sound_entry));
                        sounds.push_back(snd);
                    }
                    std::ofstream oData("sounds.cfg", std::ofstream::out);

                    for (auto& snd : sounds)
                        oData << snd.soundID << " " << snd.unk << " " << snd.frequency << std::endl;
                }


                std::cout << "INFO: Processing table 2" << std::endl;
                pFile.seekg(mst.table2Pointer, pFile.beg);

                std::vector<table2_entry> t2Data;
                {
                    for (int i = 0; i < mst.sounds; i++)
                    {
                        table2_entry t2;
                        pFile.read((char*)&t2, sizeof(table2_entry));
                        t2Data.push_back(t2);
                    }
                    std::ofstream oData("table2.cfg", std::ofstream::out);

                    for (auto& t2 : t2Data)
                        oData << t2.field0 << " " << t2.field4 << " " << t2.field8 << " " << t2.field12 << std::endl;
                }

                std::cout << "INFO: Processing table 3" << std::endl;
                pFile.seekg(mst.table3Pointer, pFile.beg);

                std::vector<table3_entry> t3Data;
                {
                    for (int i = 0; i < mst.sounds; i++)
                    {
                        table3_entry t3;
                        pFile.read((char*)&t3, sizeof(table3_entry));
                        t3Data.push_back(t3);
                    }
                    std::ofstream oData("table3.cfg", std::ofstream::out);

                    for (auto& t3 : t3Data)
                        oData << t3.field0 << " " << t3.field4 << " " << t3.field8 << " " << t3.field12 << std::endl;
                }

                std::cout << "INFO: Processing table 4" << std::endl;
                pFile.seekg(mst.table4Pointer, pFile.beg);

                std::vector<table4_entry> t4Data;
                {
                    for (int i = 0; i < mst.sounds; i++)
                    {
                        table4_entry t4;
                        pFile.read((char*)&t4, sizeof(table4_entry));
                        t4Data.push_back(t4);
                    }
                    std::ofstream oData("table4.cfg", std::ofstream::out);

                    for (auto& t4 : t4Data)
                        oData << t4.soundID<< " " << t4.field2 << " " << t4.field4 << " " << t4.field8 << " " << t4.field12 << " " << t4.field16 << " " << t4.field20 << " " << t4.field24 << " "
                        << t4.field28 << " " << t4.field32 << " " << t4.field36 << " " << t4.field40 << " " << t4.field44 << " " << t4.field48 << " " << t4.field52 << " " << t4.field56 << " "
                        << t4.field60 << " " << t4.field64 << " " << t4.field68 << " " << t4.field72 << " " << t4.field76 << " "
                        << std::endl;
                }
            }
        }
        else
        {
            std::cout << "ERROR: Failed to open " << input << std::endl;
            return 0;
        }
    }
    else
    {
       bool _t_switch = false;
       bool _l_switch = false;
       // params
       for (int i = 1; i < argc - 1; i++)
       {
           if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
               return 1;
           }
           switch (argv[i][1])
           {
           case 't': _t_switch = true;
               break;
           case 'l': _l_switch = true;
               break;
           default:
               std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
               return 0;
               break;
           }
           if (_t_switch)
           {
               std::cout << "INFO: Generating MST file" << std::endl;
               std::string output = input + ".mst";

               std::ofstream oFile(output, std::ofstream::binary);

               mst_header mst = {};
               mst.header = 0xB;
               mst.field4 = 51;

               std::vector<sound_entry> sounds;
               std::vector<table2_entry> table2;
               std::vector<table3_entry> table3;
               std::vector<table4_entry> table4;
               std::filesystem::current_path(input);


               {
                   {
                       FILE* pFile;
                       fopen_s(&pFile, "sounds.cfg", "rb");

                       if (pFile)
                       {
                           char szLine[512] = {};

                           while (fgets(szLine, sizeof(szLine), pFile))
                           {
                               if (szLine[0] == ';' || szLine[0] == ';' || szLine[0] == '\n')
                                   continue;

                               sound_entry snd = {};

                               sscanf_s(szLine, "%d %d %d", &snd.soundID, &snd.unk, &snd.frequency);

                               sounds.push_back(snd);
                           }
                           fclose(pFile);
                       }
                       else
                       {
                           std::cout << "ERROR: Could not open \"sounds.cfg\"!" << std::endl;
                           return 0;
                       }
                   }

                   {
                       FILE* pFile;
                       fopen_s(&pFile, "table2.cfg", "rb");

                       if (pFile)
                       {
                           char szLine[512] = {};

                           while (fgets(szLine, sizeof(szLine), pFile))
                           {
                               if (szLine[0] == ';' || szLine[0] == ';' || szLine[0] == '\n')
                                   continue;

                               table2_entry t2 = {};

                               sscanf_s(szLine, "%d %d %d %d", &t2.field0, &t2.field4, &t2.field8, &t2.field12);

                               table2.push_back(t2);
                           }
                           fclose(pFile);
                       }
                       else
                       {
                           std::cout << "ERROR: Could not open \"table2.cfg\"!" << std::endl;
                           return 0;
                       }
                   }

                   {
                       FILE* pFile;
                       fopen_s(&pFile, "table3.cfg", "rb");

                       if (pFile)
                       {
                           char szLine[512] = {};

                           while (fgets(szLine, sizeof(szLine), pFile))
                           {
                               if (szLine[0] == ';' || szLine[0] == ';' || szLine[0] == '\n')
                                   continue;

                               table3_entry t3 = {};

                               sscanf_s(szLine, "%d %d %d %d", &t3.field0, &t3.field4, &t3.field8, &t3.field12);

                               table3.push_back(t3);
                           }
                           fclose(pFile);
                       }
                       else
                       {
                           std::cout << "ERROR: Could not open \"table3.cfg\"!" << std::endl;
                           return 0;
                       }
                   }

                   {
                       FILE* pFile;
                       fopen_s(&pFile, "table4.cfg", "rb");

                       if (pFile)
                       {
                           char szLine[512] = {};

                           while (fgets(szLine, sizeof(szLine), pFile))
                           {
                               if (szLine[0] == ';' || szLine[0] == ';' || szLine[0] == '\n')
                                   continue;

                               table4_entry t4 = {};

                               sscanf_s(szLine, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                                   &t4.soundID, &t4.field2, &t4.field4, &t4.field8,
                                   &t4.field12, &t4.field16, &t4.field20, &t4.field24,
                                   &t4.field28, &t4.field32, &t4.field36, &t4.field40,
                                   &t4.field44, &t4.field48, &t4.field52, &t4.field56,
                                   &t4.field60, &t4.field64, &t4.field68, &t4.field72,
                                   &t4.field76);

                               table4.push_back(t4);
                           }
                           fclose(pFile);
                       }
                       else
                       {
                           std::cout << "ERROR: Could not open \"table4.cfg\"!" << std::endl;
                           return 0;
                       }
                   }

               }

               // designed for announcr mostly
               int soundsSize = sizeof(sound_entry) * sounds.size();
               int table2Size = sizeof(table2_entry) * table2.size();
               int table3Size = sizeof(table3_entry) * sounds.size();
               int table4Size = sizeof(table4_entry) * sounds.size();
               int listSize = sizeof(short) * sounds.size();
               mst.sounds = sounds.size();
               mst.table2Entries = sounds.size();
               mst.table3Entries = mst.sounds;


               int pointer = sizeof(mst_header);
               mst.soundsPointer = pointer;
               pointer += makePad(soundsSize, MST_PAD_VALUE);
               mst.table2Pointer = pointer;
               pointer += makePad(table2Size, MST_PAD_VALUE);
               mst.table3Pointer = pointer;
               pointer += makePad(table3Size, MST_PAD_VALUE);
               mst.table4Pointer = pointer;
               pointer += makePad(table4Size, MST_PAD_VALUE);
               mst.listPointer = pointer;
               pointer += makePad(listSize, MST_PAD_VALUE);
               mst.fileSize[0] = pointer;
               mst.fileSize[1] = pointer;
               oFile.write((char*)&mst, sizeof(mst_header));

               // write sounds

               for (auto& snd : sounds)
                   oFile.write((char*)&snd, sizeof(sound_entry));

               // write sounds pad
               int padSize = makePad(soundsSize, MST_PAD_VALUE) - soundsSize;
               std::unique_ptr<char[]> padBuff = std::make_unique<char[]>(padSize);
               oFile.write(padBuff.get(), padSize);

               // write t2

               for (auto& t2 : table2)
                   oFile.write((char*)&t2, sizeof(table2_entry));

               // write t2 pad
               padSize = makePad(table2Size, MST_PAD_VALUE) - table2Size;
               padBuff = std::make_unique<char[]>(padSize);
               oFile.write(padBuff.get(), padSize);

               // write t3

               for (auto& t3 : table3)
                   oFile.write((char*)&t3, sizeof(table3_entry));

               // write t3 pad
               padSize = makePad(table3Size, MST_PAD_VALUE) - table3Size;
               padBuff = std::make_unique<char[]>(padSize);
               oFile.write(padBuff.get(), padSize);

               // write t4

               for (auto& t4 : table4)
                   oFile.write((char*)&t4, sizeof(table4_entry));

               // write t4 pad
               padSize = makePad(table4Size, MST_PAD_VALUE) - table4Size;
               padBuff = std::make_unique<char[]>(padSize);
               oFile.write(padBuff.get(), padSize);

               // write list

               for (int i = 0; i < sounds.size(); i++)
               {
                   short id = i + 1;
                   oFile.write((char*)&id, sizeof(short));
               }
               // write list pad
               padSize = makePad(listSize, MST_PAD_VALUE) - listSize;
               padBuff = std::make_unique<char[]>(padSize);
               oFile.write(padBuff.get(), padSize);


           }
           else if (_l_switch)
           {
               std::cout << "INFO: Generating MSL file" << std::endl;

               std::string output = input + ".ms2";
               std::string file = input + ".txt";

               std::ofstream oFile(output, std::ofstream::binary);

              struct msl_entry {
                   msl_file file;
                   char path[260] = {};
               };

               std::vector<msl_entry> files;
               std::vector<int> sizes;
               std::vector<std::string> bankNames;

               int numSounds = 0;
               int numBanks = 0;

               std::filesystem::current_path(input);

               FILE* pFile;
               fopen_s(&pFile, file.c_str(), "rb");

               if (pFile)
               {
                   char szLine[512] = {};

                   while (fgets(szLine, sizeof(szLine), pFile))
                   {
                       if (szLine[0] == ';' || szLine[0] == ';' || szLine[0] == '\n')
                           continue;

                       msl_entry ent = {};
                       sscanf_s(szLine, "%d %d %s", &ent.file.id, &ent.file.field10, ent.path);

                       files.push_back(ent);
                   }
                   fclose(pFile);
               }
               else
               {
                   std::cout << "ERROR: Could not open " << file << "!" << std::endl;
                   return 0;
               }

               for (auto& file : files)
               {
                   int size = std::filesystem::file_size(file.path);
                   sizes.push_back(size);

                   std::string name = file.path;
                   name = name.substr(name.find_last_of("\\") + 1);

                   std::string extension = name.substr(name.find_last_of(".") + 1);

                   if (extension == "mst")
                   {
                       numBanks++;
                       bankNames.push_back(name);
                   }
                   else if (extension == "sfx")
                       numSounds++;
               }
               // generate header

               int numFiles = numBanks + numSounds;

               msl_header msl = {};
               msl.field0 = 1;
               msl.files = numFiles + 2;
               msl.field12 = 0;
               msl.field20 = 0;
               msl.field24 = 1;


               int stringSize = 0;

               for (unsigned int i = 0; i < bankNames.size(); i++)
                   stringSize += bankNames[i].length() + 1;

               int entrySize = numFiles * sizeof(msl_file);
               int padSize = 9;

               int filesSize = 0;
               for (unsigned int i = 0; i < sizes.size(); i++)
                   filesSize += sizes[i];


               msl.fileSize = sizeof(msl_header) + entrySize + padSize + stringSize + filesSize;

               msl.endOffset = (sizeof(int) * 4) + entrySize + padSize + stringSize;

               oFile.write((char*)&msl, sizeof(msl_header));


               int baseOffset = entrySize + padSize + stringSize + sizeof(msl_header);
               for (unsigned int i = 0; i < files.size(); i++)
               {
                   files[i].file.offset = baseOffset;
                   files[i].file.size = sizes[i];
                   oFile.write((char*)&files[i].file, sizeof(msl_file));
                   baseOffset += sizes[i];
               }

               char pad[9] = {};
               oFile.write((char*)&pad, sizeof(pad));

               for (unsigned int i = 0; i < bankNames.size(); i++)
               {
                   oFile.write((char*)&bankNames[i].c_str()[0], bankNames[i].length() + 1);
               }


               for (unsigned int i = 0; i < files.size(); i++)
               {
                   std::ifstream pFile(files[i].path, std::ifstream::binary);
                   std::cout << "Processing: " << files[i].path << std::endl;
                   if (!pFile)
                   {
                       std::cout << "ERROR: Could not open " << files[i].path << "!" << std::endl;
                       return 0;
                   }

                   int dataSize = sizes[i];
                   std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
                   pFile.read(dataBuff.get(), dataSize);
                   oFile.write(dataBuff.get(), dataSize);
               }

               std::cout << "Banks: " << numBanks << " Sounds: " << numSounds << std::endl;
               std::cout << "Finished." << std::endl;
           }

        
       }
    }


   
    return 1;
}

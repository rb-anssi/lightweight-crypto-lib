/*------------------------ MIT License HEADER ------------------------------------
    Copyright ANSSI and NTU (2015)
    Contributors:
    Ryad BENADJILA [ryad.benadjila@ssi.gouv.fr] and
    Jian GUO [ntu.guo@gmail.com] and
    Victor LOMNE [victor.lomne@ssi.gouv.fr] and
    Thomas PEYRIN [thomas.peyrin@gmail.com]

    This software is a computer program whose purpose is to implement
    lightweight block ciphers with different optimizations for the x86
    platform. Three algorithms have been implemented: PRESENT, LED and 
    Piccolo. Three techniques have been explored: table based 
    implementations, vperm (for vector permutation) and bitslice 
    implementations. For more details, please refer to the SAC 2013
    paper:
    http://eprint.iacr.org/2013/445
    as well as the documentation of the project.
    Here is a big picture of how the code is divided:
      - src/common contains common headers, structures and functions.
      - src/table contains table based implementations, with the code 
        that generates the tables in src/table/gen_tables. The code here 
        is written in pure C so it should compile on any platform (x86  
        and other architectures), as well as any OS flavour (*nix, 
        Windows ...).
      - src/vperm contains vperm based implementations. They are written 
        in inline assembly for x86_64 and will only compile and work on 
        this platform. The code only compiles with gcc, but porting it to
        other assembly flavours should not be too complicated.
      - src/bitslice contains bitslice based implementations. They are 
        written in asm intrinsics. It should compile and run on i386 as 
        well as x86_64 platforms, and it should be portable to other OS 
        flavours since intrinsics are standard among many compilers.
    Note: vperm and bitslice implementations require a x86 CPU with at least 
    SSSE3 extensions.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

    Except as contained in this notice, the name(s) of the above copyright holders
    shall not be used in advertising or otherwise to promote the sale, use or other
    dealings in this Software without prior written authorization.


-------------------------- MIT License HEADER ----------------------------------*/
unsigned int T0_Piccolo[256] = {0x0fe1, 0xa536, 0x5a1b, 0xc39a, 0xd2a8, 0x694d, 0xe1fe, 0x787f, 0xf0cc, 0x4b29, 0x9660, 0x1ed3, 0x8752, 0x2d85, 0xb404, 0x3cb7, 0xa29b, 0x084c, 0xf761, 0x6ee0, 0x7fd2, 0xc437, 0x4c84, 0xd505, 0x5db6, 0xe653, 0x3b1a, 0xb3a9, 0x2a28, 0x80ff, 0x197e, 0x91cd, 0x5044, 0xfa93, 0x05be, 0x9c3f, 0x8d0d, 0x36e8, 0xbe5b, 0x27da, 0xaf69, 0x148c, 0xc9c5, 0x4176, 0xd8f7, 0x7220, 0xeba1, 0x6312, 0xc85d, 0x628a, 0x9da7, 0x0426, 0x1514, 0xaef1, 0x2642, 0xbfc3, 0x3770, 0x8c95, 0x51dc, 0xd96f, 0x40ee, 0xea39, 0x73b8, 0xfb0b, 0xdb7c, 0x71ab, 0x8e86, 0x1707, 0x0635, 0xbdd0, 0x3563, 0xace2, 0x2451, 0x9fb4, 0x42fd, 0xca4e, 0x53cf, 0xf918, 0x6099, 0xe82a, 0x6527, 0xcff0, 0x30dd, 0xa95c, 0xb86e, 0x038b, 0x8b38, 0x12b9, 0x9a0a, 0x21ef, 0xfca6, 0x7415, 0xed94, 0x4743, 0xdec2, 0x5671, 0xee1f, 0x44c8, 0xbbe5, 0x2264, 0x3356, 0x88b3, 0x0000, 0x9981, 0x1132, 0xaad7, 0x779e, 0xff2d, 0x66ac, 0xcc7b, 0x55fa, 0xdd49, 0x7606, 0xdcd1, 0x23fc, 0xba7d, 0xab4f, 0x10aa, 0x9819, 0x0198, 0x892b, 0x32ce, 0xef87, 0x6734, 0xfeb5, 0x5462, 0xcde3, 0x4550, 0xfd3e, 0x57e9, 0xa8c4, 0x3145, 0x2077, 0x9b92, 0x1321, 0x8aa0, 0x0213, 0xb9f6, 0x64bf, 0xec0c, 0x758d, 0xdf5a, 0x46db, 0xce68, 0x4365, 0xe9b2, 0x169f, 0x8f1e, 0x9e2c, 0x25c9, 0xad7a, 0x34fb, 0xbc48, 0x07ad, 0xdae4, 0x5257, 0xcbd6, 0x6101, 0xf880, 0x7033, 0x97f8, 0x3d2f, 0xc202, 0x5b83, 0x4ab1, 0xf154, 0x79e7, 0xe066, 0x68d5, 0xd330, 0x0e79, 0x86ca, 0x1f4b, 0xb59c, 0x2c1d, 0xa4ae, 0x1cc0, 0xb617, 0x493a, 0xd0bb, 0xc189, 0x7a6c, 0xf2df, 0x6b5e, 0xe3ed, 0x5808, 0x8541, 0x0df2, 0x9473, 0x3ea4, 0xa725, 0x2f96, 0x84d9, 0x2e0e, 0xd123, 0x48a2, 0x5990, 0xe275, 0x6ac6, 0xf347, 0x7bf4, 0xc011, 0x1d58, 0x95eb, 0x0c6a, 0xa6bd, 0x3f3c, 0xb78f, 0x29a3, 0x8374, 0x7c59, 0xe5d8, 0xf4ea, 0x4f0f, 0xc7bc, 0x5e3d, 0xd68e, 0x6d6b, 0xb022, 0x3891, 0xa110, 0x0bc7, 0x9246, 0x1af5, 0xb1ba, 0x1b6d, 0xe440, 0x7dc1, 0x6cf3, 0xd716, 0x5fa5, 0xc624, 0x4e97, 0xf572, 0x283b, 0xa088, 0x3909, 0x93de, 0x0a5f, 0x82ec, 0x3a82, 0x9055, 0x6f78, 0xf6f9, 0xe7cb, 0x5c2e, 0xd49d, 0x4d1c, 0xc5af, 0x7e4a, 0xa303, 0x2bb0, 0xb231, 0x18e6, 0x8167, 0x09d4};

unsigned int T1_Piccolo[256] = {0xe10f, 0x36a5, 0x1b5a, 0x9ac3, 0xa8d2, 0x4d69, 0xfee1, 0x7f78, 0xccf0, 0x294b, 0x6096, 0xd31e, 0x5287, 0x852d, 0x04b4, 0xb73c, 0x9ba2, 0x4c08, 0x61f7, 0xe06e, 0xd27f, 0x37c4, 0x844c, 0x05d5, 0xb65d, 0x53e6, 0x1a3b, 0xa9b3, 0x282a, 0xff80, 0x7e19, 0xcd91, 0x4450, 0x93fa, 0xbe05, 0x3f9c, 0x0d8d, 0xe836, 0x5bbe, 0xda27, 0x69af, 0x8c14, 0xc5c9, 0x7641, 0xf7d8, 0x2072, 0xa1eb, 0x1263, 0x5dc8, 0x8a62, 0xa79d, 0x2604, 0x1415, 0xf1ae, 0x4226, 0xc3bf, 0x7037, 0x958c, 0xdc51, 0x6fd9, 0xee40, 0x39ea, 0xb873, 0x0bfb, 0x7cdb, 0xab71, 0x868e, 0x0717, 0x3506, 0xd0bd, 0x6335, 0xe2ac, 0x5124, 0xb49f, 0xfd42, 0x4eca, 0xcf53, 0x18f9, 0x9960, 0x2ae8, 0x2765, 0xf0cf, 0xdd30, 0x5ca9, 0x6eb8, 0x8b03, 0x388b, 0xb912, 0x0a9a, 0xef21, 0xa6fc, 0x1574, 0x94ed, 0x4347, 0xc2de, 0x7156, 0x1fee, 0xc844, 0xe5bb, 0x6422, 0x5633, 0xb388, 0x0000, 0x8199, 0x3211, 0xd7aa, 0x9e77, 0x2dff, 0xac66, 0x7bcc, 0xfa55, 0x49dd, 0x0676, 0xd1dc, 0xfc23, 0x7dba, 0x4fab, 0xaa10, 0x1998, 0x9801, 0x2b89, 0xce32, 0x87ef, 0x3467, 0xb5fe, 0x6254, 0xe3cd, 0x5045, 0x3efd, 0xe957, 0xc4a8, 0x4531, 0x7720, 0x929b, 0x2113, 0xa08a, 0x1302, 0xf6b9, 0xbf64, 0x0cec, 0x8d75, 0x5adf, 0xdb46, 0x68ce, 0x6543, 0xb2e9, 0x9f16, 0x1e8f, 0x2c9e, 0xc925, 0x7aad, 0xfb34, 0x48bc, 0xad07, 0xe4da, 0x5752, 0xd6cb, 0x0161, 0x80f8, 0x3370, 0xf897, 0x2f3d, 0x02c2, 0x835b, 0xb14a, 0x54f1, 0xe779, 0x66e0, 0xd568, 0x30d3, 0x790e, 0xca86, 0x4b1f, 0x9cb5, 0x1d2c, 0xaea4, 0xc01c, 0x17b6, 0x3a49, 0xbbd0, 0x89c1, 0x6c7a, 0xdff2, 0x5e6b, 0xede3, 0x0858, 0x4185, 0xf20d, 0x7394, 0xa43e, 0x25a7, 0x962f, 0xd984, 0x0e2e, 0x23d1, 0xa248, 0x9059, 0x75e2, 0xc66a, 0x47f3, 0xf47b, 0x11c0, 0x581d, 0xeb95, 0x6a0c, 0xbda6, 0x3c3f, 0x8fb7, 0xa329, 0x7483, 0x597c, 0xd8e5, 0xeaf4, 0x0f4f, 0xbcc7, 0x3d5e, 0x8ed6, 0x6b6d, 0x22b0, 0x9138, 0x10a1, 0xc70b, 0x4692, 0xf51a, 0xbab1, 0x6d1b, 0x40e4, 0xc17d, 0xf36c, 0x16d7, 0xa55f, 0x24c6, 0x974e, 0x72f5, 0x3b28, 0x88a0, 0x0939, 0xde93, 0x5f0a, 0xec82, 0x823a, 0x5590, 0x786f, 0xf9f6, 0xcbe7, 0x2e5c, 0x9dd4, 0x1c4d, 0xafc5, 0x4a7e, 0x03a3, 0xb02b, 0x31b2, 0xe618, 0x6781, 0xd409};

unsigned long long T2_Piccolo[256] = {0x0000000000ee0000, 0x0000000000e40000, 0x0000000000eb0000, 0x0000000000e20000, 0x0000000000e30000, 0x0000000000e80000, 0x0000000000e00000, 0x0000000000e90000, 0x0000000000e10000, 0x0000000000ea0000, 0x0000000000e70000, 0x0000000000ef0000, 0x0000000000e60000, 0x0000000000ec0000, 0x0000000000e50000, 0x0000000000ed0000, 0x00000000004e0000, 0x0000000000440000, 0x00000000004b0000, 0x0000000000420000, 0x0000000000430000, 0x0000000000480000, 0x0000000000400000, 0x0000000000490000, 0x0000000000410000, 0x00000000004a0000, 0x0000000000470000, 0x00000000004f0000, 0x0000000000460000, 0x00000000004c0000, 0x0000000000450000, 0x00000000004d0000, 0x0000000000be0000, 0x0000000000b40000, 0x0000000000bb0000, 0x0000000000b20000, 0x0000000000b30000, 0x0000000000b80000, 0x0000000000b00000, 0x0000000000b90000, 0x0000000000b10000, 0x0000000000ba0000, 0x0000000000b70000, 0x0000000000bf0000, 0x0000000000b60000, 0x0000000000bc0000, 0x0000000000b50000, 0x0000000000bd0000, 0x00000000002e0000, 0x0000000000240000, 0x00000000002b0000, 0x0000000000220000, 0x0000000000230000, 0x0000000000280000, 0x0000000000200000, 0x0000000000290000, 0x0000000000210000, 0x00000000002a0000, 0x0000000000270000, 0x00000000002f0000, 0x0000000000260000, 0x00000000002c0000, 0x0000000000250000, 0x00000000002d0000, 0x00000000003e0000, 0x0000000000340000, 0x00000000003b0000, 0x0000000000320000, 0x0000000000330000, 0x0000000000380000, 0x0000000000300000, 0x0000000000390000, 0x0000000000310000, 0x00000000003a0000, 0x0000000000370000, 0x00000000003f0000, 0x0000000000360000, 0x00000000003c0000, 0x0000000000350000, 0x00000000003d0000, 0x00000000008e0000, 0x0000000000840000, 0x00000000008b0000, 0x0000000000820000, 0x0000000000830000, 0x0000000000880000, 0x0000000000800000, 0x0000000000890000, 0x0000000000810000, 0x00000000008a0000, 0x0000000000870000, 0x00000000008f0000, 0x0000000000860000, 0x00000000008c0000, 0x0000000000850000, 0x00000000008d0000, 0x00000000000e0000, 0x0000000000040000, 0x00000000000b0000, 0x0000000000020000, 0x0000000000030000, 0x0000000000080000, 0x0000000000000000, 0x0000000000090000, 0x0000000000010000, 0x00000000000a0000, 0x0000000000070000, 0x00000000000f0000, 0x0000000000060000, 0x00000000000c0000, 0x0000000000050000, 0x00000000000d0000, 0x00000000009e0000, 0x0000000000940000, 0x00000000009b0000, 0x0000000000920000, 0x0000000000930000, 0x0000000000980000, 0x0000000000900000, 0x0000000000990000, 0x0000000000910000, 0x00000000009a0000, 0x0000000000970000, 0x00000000009f0000, 0x0000000000960000, 0x00000000009c0000, 0x0000000000950000, 0x00000000009d0000, 0x00000000001e0000, 0x0000000000140000, 0x00000000001b0000, 0x0000000000120000, 0x0000000000130000, 0x0000000000180000, 0x0000000000100000, 0x0000000000190000, 0x0000000000110000, 0x00000000001a0000, 0x0000000000170000, 0x00000000001f0000, 0x0000000000160000, 0x00000000001c0000, 0x0000000000150000, 0x00000000001d0000, 0x0000000000ae0000, 0x0000000000a40000, 0x0000000000ab0000, 0x0000000000a20000, 0x0000000000a30000, 0x0000000000a80000, 0x0000000000a00000, 0x0000000000a90000, 0x0000000000a10000, 0x0000000000aa0000, 0x0000000000a70000, 0x0000000000af0000, 0x0000000000a60000, 0x0000000000ac0000, 0x0000000000a50000, 0x0000000000ad0000, 0x00000000007e0000, 0x0000000000740000, 0x00000000007b0000, 0x0000000000720000, 0x0000000000730000, 0x0000000000780000, 0x0000000000700000, 0x0000000000790000, 0x0000000000710000, 0x00000000007a0000, 0x0000000000770000, 0x00000000007f0000, 0x0000000000760000, 0x00000000007c0000, 0x0000000000750000, 0x00000000007d0000, 0x0000000000fe0000, 0x0000000000f40000, 0x0000000000fb0000, 0x0000000000f20000, 0x0000000000f30000, 0x0000000000f80000, 0x0000000000f00000, 0x0000000000f90000, 0x0000000000f10000, 0x0000000000fa0000, 0x0000000000f70000, 0x0000000000ff0000, 0x0000000000f60000, 0x0000000000fc0000, 0x0000000000f50000, 0x0000000000fd0000, 0x00000000006e0000, 0x0000000000640000, 0x00000000006b0000, 0x0000000000620000, 0x0000000000630000, 0x0000000000680000, 0x0000000000600000, 0x0000000000690000, 0x0000000000610000, 0x00000000006a0000, 0x0000000000670000, 0x00000000006f0000, 0x0000000000660000, 0x00000000006c0000, 0x0000000000650000, 0x00000000006d0000, 0x0000000000ce0000, 0x0000000000c40000, 0x0000000000cb0000, 0x0000000000c20000, 0x0000000000c30000, 0x0000000000c80000, 0x0000000000c00000, 0x0000000000c90000, 0x0000000000c10000, 0x0000000000ca0000, 0x0000000000c70000, 0x0000000000cf0000, 0x0000000000c60000, 0x0000000000cc0000, 0x0000000000c50000, 0x0000000000cd0000, 0x00000000005e0000, 0x0000000000540000, 0x00000000005b0000, 0x0000000000520000, 0x0000000000530000, 0x0000000000580000, 0x0000000000500000, 0x0000000000590000, 0x0000000000510000, 0x00000000005a0000, 0x0000000000570000, 0x00000000005f0000, 0x0000000000560000, 0x00000000005c0000, 0x0000000000550000, 0x00000000005d0000, 0x0000000000de0000, 0x0000000000d40000, 0x0000000000db0000, 0x0000000000d20000, 0x0000000000d30000, 0x0000000000d80000, 0x0000000000d00000, 0x0000000000d90000, 0x0000000000d10000, 0x0000000000da0000, 0x0000000000d70000, 0x0000000000df0000, 0x0000000000d60000, 0x0000000000dc0000, 0x0000000000d50000, 0x0000000000dd0000};

unsigned long long T3_Piccolo[256] = {0x00000000ee000000, 0x00000000e4000000, 0x00000000eb000000, 0x00000000e2000000, 0x00000000e3000000, 0x00000000e8000000, 0x00000000e0000000, 0x00000000e9000000, 0x00000000e1000000, 0x00000000ea000000, 0x00000000e7000000, 0x00000000ef000000, 0x00000000e6000000, 0x00000000ec000000, 0x00000000e5000000, 0x00000000ed000000, 0x000000004e000000, 0x0000000044000000, 0x000000004b000000, 0x0000000042000000, 0x0000000043000000, 0x0000000048000000, 0x0000000040000000, 0x0000000049000000, 0x0000000041000000, 0x000000004a000000, 0x0000000047000000, 0x000000004f000000, 0x0000000046000000, 0x000000004c000000, 0x0000000045000000, 0x000000004d000000, 0x00000000be000000, 0x00000000b4000000, 0x00000000bb000000, 0x00000000b2000000, 0x00000000b3000000, 0x00000000b8000000, 0x00000000b0000000, 0x00000000b9000000, 0x00000000b1000000, 0x00000000ba000000, 0x00000000b7000000, 0x00000000bf000000, 0x00000000b6000000, 0x00000000bc000000, 0x00000000b5000000, 0x00000000bd000000, 0x000000002e000000, 0x0000000024000000, 0x000000002b000000, 0x0000000022000000, 0x0000000023000000, 0x0000000028000000, 0x0000000020000000, 0x0000000029000000, 0x0000000021000000, 0x000000002a000000, 0x0000000027000000, 0x000000002f000000, 0x0000000026000000, 0x000000002c000000, 0x0000000025000000, 0x000000002d000000, 0x000000003e000000, 0x0000000034000000, 0x000000003b000000, 0x0000000032000000, 0x0000000033000000, 0x0000000038000000, 0x0000000030000000, 0x0000000039000000, 0x0000000031000000, 0x000000003a000000, 0x0000000037000000, 0x000000003f000000, 0x0000000036000000, 0x000000003c000000, 0x0000000035000000, 0x000000003d000000, 0x000000008e000000, 0x0000000084000000, 0x000000008b000000, 0x0000000082000000, 0x0000000083000000, 0x0000000088000000, 0x0000000080000000, 0x0000000089000000, 0x0000000081000000, 0x000000008a000000, 0x0000000087000000, 0x000000008f000000, 0x0000000086000000, 0x000000008c000000, 0x0000000085000000, 0x000000008d000000, 0x000000000e000000, 0x0000000004000000, 0x000000000b000000, 0x0000000002000000, 0x0000000003000000, 0x0000000008000000, 0x0000000000000000, 0x0000000009000000, 0x0000000001000000, 0x000000000a000000, 0x0000000007000000, 0x000000000f000000, 0x0000000006000000, 0x000000000c000000, 0x0000000005000000, 0x000000000d000000, 0x000000009e000000, 0x0000000094000000, 0x000000009b000000, 0x0000000092000000, 0x0000000093000000, 0x0000000098000000, 0x0000000090000000, 0x0000000099000000, 0x0000000091000000, 0x000000009a000000, 0x0000000097000000, 0x000000009f000000, 0x0000000096000000, 0x000000009c000000, 0x0000000095000000, 0x000000009d000000, 0x000000001e000000, 0x0000000014000000, 0x000000001b000000, 0x0000000012000000, 0x0000000013000000, 0x0000000018000000, 0x0000000010000000, 0x0000000019000000, 0x0000000011000000, 0x000000001a000000, 0x0000000017000000, 0x000000001f000000, 0x0000000016000000, 0x000000001c000000, 0x0000000015000000, 0x000000001d000000, 0x00000000ae000000, 0x00000000a4000000, 0x00000000ab000000, 0x00000000a2000000, 0x00000000a3000000, 0x00000000a8000000, 0x00000000a0000000, 0x00000000a9000000, 0x00000000a1000000, 0x00000000aa000000, 0x00000000a7000000, 0x00000000af000000, 0x00000000a6000000, 0x00000000ac000000, 0x00000000a5000000, 0x00000000ad000000, 0x000000007e000000, 0x0000000074000000, 0x000000007b000000, 0x0000000072000000, 0x0000000073000000, 0x0000000078000000, 0x0000000070000000, 0x0000000079000000, 0x0000000071000000, 0x000000007a000000, 0x0000000077000000, 0x000000007f000000, 0x0000000076000000, 0x000000007c000000, 0x0000000075000000, 0x000000007d000000, 0x00000000fe000000, 0x00000000f4000000, 0x00000000fb000000, 0x00000000f2000000, 0x00000000f3000000, 0x00000000f8000000, 0x00000000f0000000, 0x00000000f9000000, 0x00000000f1000000, 0x00000000fa000000, 0x00000000f7000000, 0x00000000ff000000, 0x00000000f6000000, 0x00000000fc000000, 0x00000000f5000000, 0x00000000fd000000, 0x000000006e000000, 0x0000000064000000, 0x000000006b000000, 0x0000000062000000, 0x0000000063000000, 0x0000000068000000, 0x0000000060000000, 0x0000000069000000, 0x0000000061000000, 0x000000006a000000, 0x0000000067000000, 0x000000006f000000, 0x0000000066000000, 0x000000006c000000, 0x0000000065000000, 0x000000006d000000, 0x00000000ce000000, 0x00000000c4000000, 0x00000000cb000000, 0x00000000c2000000, 0x00000000c3000000, 0x00000000c8000000, 0x00000000c0000000, 0x00000000c9000000, 0x00000000c1000000, 0x00000000ca000000, 0x00000000c7000000, 0x00000000cf000000, 0x00000000c6000000, 0x00000000cc000000, 0x00000000c5000000, 0x00000000cd000000, 0x000000005e000000, 0x0000000054000000, 0x000000005b000000, 0x0000000052000000, 0x0000000053000000, 0x0000000058000000, 0x0000000050000000, 0x0000000059000000, 0x0000000051000000, 0x000000005a000000, 0x0000000057000000, 0x000000005f000000, 0x0000000056000000, 0x000000005c000000, 0x0000000055000000, 0x000000005d000000, 0x00000000de000000, 0x00000000d4000000, 0x00000000db000000, 0x00000000d2000000, 0x00000000d3000000, 0x00000000d8000000, 0x00000000d0000000, 0x00000000d9000000, 0x00000000d1000000, 0x00000000da000000, 0x00000000d7000000, 0x00000000df000000, 0x00000000d6000000, 0x00000000dc000000, 0x00000000d5000000, 0x00000000dd000000};

unsigned long long T4_Piccolo[256] = {0x00ee000000000000, 0x00e4000000000000, 0x00eb000000000000, 0x00e2000000000000, 0x00e3000000000000, 0x00e8000000000000, 0x00e0000000000000, 0x00e9000000000000, 0x00e1000000000000, 0x00ea000000000000, 0x00e7000000000000, 0x00ef000000000000, 0x00e6000000000000, 0x00ec000000000000, 0x00e5000000000000, 0x00ed000000000000, 0x004e000000000000, 0x0044000000000000, 0x004b000000000000, 0x0042000000000000, 0x0043000000000000, 0x0048000000000000, 0x0040000000000000, 0x0049000000000000, 0x0041000000000000, 0x004a000000000000, 0x0047000000000000, 0x004f000000000000, 0x0046000000000000, 0x004c000000000000, 0x0045000000000000, 0x004d000000000000, 0x00be000000000000, 0x00b4000000000000, 0x00bb000000000000, 0x00b2000000000000, 0x00b3000000000000, 0x00b8000000000000, 0x00b0000000000000, 0x00b9000000000000, 0x00b1000000000000, 0x00ba000000000000, 0x00b7000000000000, 0x00bf000000000000, 0x00b6000000000000, 0x00bc000000000000, 0x00b5000000000000, 0x00bd000000000000, 0x002e000000000000, 0x0024000000000000, 0x002b000000000000, 0x0022000000000000, 0x0023000000000000, 0x0028000000000000, 0x0020000000000000, 0x0029000000000000, 0x0021000000000000, 0x002a000000000000, 0x0027000000000000, 0x002f000000000000, 0x0026000000000000, 0x002c000000000000, 0x0025000000000000, 0x002d000000000000, 0x003e000000000000, 0x0034000000000000, 0x003b000000000000, 0x0032000000000000, 0x0033000000000000, 0x0038000000000000, 0x0030000000000000, 0x0039000000000000, 0x0031000000000000, 0x003a000000000000, 0x0037000000000000, 0x003f000000000000, 0x0036000000000000, 0x003c000000000000, 0x0035000000000000, 0x003d000000000000, 0x008e000000000000, 0x0084000000000000, 0x008b000000000000, 0x0082000000000000, 0x0083000000000000, 0x0088000000000000, 0x0080000000000000, 0x0089000000000000, 0x0081000000000000, 0x008a000000000000, 0x0087000000000000, 0x008f000000000000, 0x0086000000000000, 0x008c000000000000, 0x0085000000000000, 0x008d000000000000, 0x000e000000000000, 0x0004000000000000, 0x000b000000000000, 0x0002000000000000, 0x0003000000000000, 0x0008000000000000, 0x0000000000000000, 0x0009000000000000, 0x0001000000000000, 0x000a000000000000, 0x0007000000000000, 0x000f000000000000, 0x0006000000000000, 0x000c000000000000, 0x0005000000000000, 0x000d000000000000, 0x009e000000000000, 0x0094000000000000, 0x009b000000000000, 0x0092000000000000, 0x0093000000000000, 0x0098000000000000, 0x0090000000000000, 0x0099000000000000, 0x0091000000000000, 0x009a000000000000, 0x0097000000000000, 0x009f000000000000, 0x0096000000000000, 0x009c000000000000, 0x0095000000000000, 0x009d000000000000, 0x001e000000000000, 0x0014000000000000, 0x001b000000000000, 0x0012000000000000, 0x0013000000000000, 0x0018000000000000, 0x0010000000000000, 0x0019000000000000, 0x0011000000000000, 0x001a000000000000, 0x0017000000000000, 0x001f000000000000, 0x0016000000000000, 0x001c000000000000, 0x0015000000000000, 0x001d000000000000, 0x00ae000000000000, 0x00a4000000000000, 0x00ab000000000000, 0x00a2000000000000, 0x00a3000000000000, 0x00a8000000000000, 0x00a0000000000000, 0x00a9000000000000, 0x00a1000000000000, 0x00aa000000000000, 0x00a7000000000000, 0x00af000000000000, 0x00a6000000000000, 0x00ac000000000000, 0x00a5000000000000, 0x00ad000000000000, 0x007e000000000000, 0x0074000000000000, 0x007b000000000000, 0x0072000000000000, 0x0073000000000000, 0x0078000000000000, 0x0070000000000000, 0x0079000000000000, 0x0071000000000000, 0x007a000000000000, 0x0077000000000000, 0x007f000000000000, 0x0076000000000000, 0x007c000000000000, 0x0075000000000000, 0x007d000000000000, 0x00fe000000000000, 0x00f4000000000000, 0x00fb000000000000, 0x00f2000000000000, 0x00f3000000000000, 0x00f8000000000000, 0x00f0000000000000, 0x00f9000000000000, 0x00f1000000000000, 0x00fa000000000000, 0x00f7000000000000, 0x00ff000000000000, 0x00f6000000000000, 0x00fc000000000000, 0x00f5000000000000, 0x00fd000000000000, 0x006e000000000000, 0x0064000000000000, 0x006b000000000000, 0x0062000000000000, 0x0063000000000000, 0x0068000000000000, 0x0060000000000000, 0x0069000000000000, 0x0061000000000000, 0x006a000000000000, 0x0067000000000000, 0x006f000000000000, 0x0066000000000000, 0x006c000000000000, 0x0065000000000000, 0x006d000000000000, 0x00ce000000000000, 0x00c4000000000000, 0x00cb000000000000, 0x00c2000000000000, 0x00c3000000000000, 0x00c8000000000000, 0x00c0000000000000, 0x00c9000000000000, 0x00c1000000000000, 0x00ca000000000000, 0x00c7000000000000, 0x00cf000000000000, 0x00c6000000000000, 0x00cc000000000000, 0x00c5000000000000, 0x00cd000000000000, 0x005e000000000000, 0x0054000000000000, 0x005b000000000000, 0x0052000000000000, 0x0053000000000000, 0x0058000000000000, 0x0050000000000000, 0x0059000000000000, 0x0051000000000000, 0x005a000000000000, 0x0057000000000000, 0x005f000000000000, 0x0056000000000000, 0x005c000000000000, 0x0055000000000000, 0x005d000000000000, 0x00de000000000000, 0x00d4000000000000, 0x00db000000000000, 0x00d2000000000000, 0x00d3000000000000, 0x00d8000000000000, 0x00d0000000000000, 0x00d9000000000000, 0x00d1000000000000, 0x00da000000000000, 0x00d7000000000000, 0x00df000000000000, 0x00d6000000000000, 0x00dc000000000000, 0x00d5000000000000, 0x00dd000000000000};

unsigned long long T5_Piccolo[256] = {0xee00000000000000, 0xe400000000000000, 0xeb00000000000000, 0xe200000000000000, 0xe300000000000000, 0xe800000000000000, 0xe000000000000000, 0xe900000000000000, 0xe100000000000000, 0xea00000000000000, 0xe700000000000000, 0xef00000000000000, 0xe600000000000000, 0xec00000000000000, 0xe500000000000000, 0xed00000000000000, 0x4e00000000000000, 0x4400000000000000, 0x4b00000000000000, 0x4200000000000000, 0x4300000000000000, 0x4800000000000000, 0x4000000000000000, 0x4900000000000000, 0x4100000000000000, 0x4a00000000000000, 0x4700000000000000, 0x4f00000000000000, 0x4600000000000000, 0x4c00000000000000, 0x4500000000000000, 0x4d00000000000000, 0xbe00000000000000, 0xb400000000000000, 0xbb00000000000000, 0xb200000000000000, 0xb300000000000000, 0xb800000000000000, 0xb000000000000000, 0xb900000000000000, 0xb100000000000000, 0xba00000000000000, 0xb700000000000000, 0xbf00000000000000, 0xb600000000000000, 0xbc00000000000000, 0xb500000000000000, 0xbd00000000000000, 0x2e00000000000000, 0x2400000000000000, 0x2b00000000000000, 0x2200000000000000, 0x2300000000000000, 0x2800000000000000, 0x2000000000000000, 0x2900000000000000, 0x2100000000000000, 0x2a00000000000000, 0x2700000000000000, 0x2f00000000000000, 0x2600000000000000, 0x2c00000000000000, 0x2500000000000000, 0x2d00000000000000, 0x3e00000000000000, 0x3400000000000000, 0x3b00000000000000, 0x3200000000000000, 0x3300000000000000, 0x3800000000000000, 0x3000000000000000, 0x3900000000000000, 0x3100000000000000, 0x3a00000000000000, 0x3700000000000000, 0x3f00000000000000, 0x3600000000000000, 0x3c00000000000000, 0x3500000000000000, 0x3d00000000000000, 0x8e00000000000000, 0x8400000000000000, 0x8b00000000000000, 0x8200000000000000, 0x8300000000000000, 0x8800000000000000, 0x8000000000000000, 0x8900000000000000, 0x8100000000000000, 0x8a00000000000000, 0x8700000000000000, 0x8f00000000000000, 0x8600000000000000, 0x8c00000000000000, 0x8500000000000000, 0x8d00000000000000, 0x0e00000000000000, 0x0400000000000000, 0x0b00000000000000, 0x0200000000000000, 0x0300000000000000, 0x0800000000000000, 0x0000000000000000, 0x0900000000000000, 0x0100000000000000, 0x0a00000000000000, 0x0700000000000000, 0x0f00000000000000, 0x0600000000000000, 0x0c00000000000000, 0x0500000000000000, 0x0d00000000000000, 0x9e00000000000000, 0x9400000000000000, 0x9b00000000000000, 0x9200000000000000, 0x9300000000000000, 0x9800000000000000, 0x9000000000000000, 0x9900000000000000, 0x9100000000000000, 0x9a00000000000000, 0x9700000000000000, 0x9f00000000000000, 0x9600000000000000, 0x9c00000000000000, 0x9500000000000000, 0x9d00000000000000, 0x1e00000000000000, 0x1400000000000000, 0x1b00000000000000, 0x1200000000000000, 0x1300000000000000, 0x1800000000000000, 0x1000000000000000, 0x1900000000000000, 0x1100000000000000, 0x1a00000000000000, 0x1700000000000000, 0x1f00000000000000, 0x1600000000000000, 0x1c00000000000000, 0x1500000000000000, 0x1d00000000000000, 0xae00000000000000, 0xa400000000000000, 0xab00000000000000, 0xa200000000000000, 0xa300000000000000, 0xa800000000000000, 0xa000000000000000, 0xa900000000000000, 0xa100000000000000, 0xaa00000000000000, 0xa700000000000000, 0xaf00000000000000, 0xa600000000000000, 0xac00000000000000, 0xa500000000000000, 0xad00000000000000, 0x7e00000000000000, 0x7400000000000000, 0x7b00000000000000, 0x7200000000000000, 0x7300000000000000, 0x7800000000000000, 0x7000000000000000, 0x7900000000000000, 0x7100000000000000, 0x7a00000000000000, 0x7700000000000000, 0x7f00000000000000, 0x7600000000000000, 0x7c00000000000000, 0x7500000000000000, 0x7d00000000000000, 0xfe00000000000000, 0xf400000000000000, 0xfb00000000000000, 0xf200000000000000, 0xf300000000000000, 0xf800000000000000, 0xf000000000000000, 0xf900000000000000, 0xf100000000000000, 0xfa00000000000000, 0xf700000000000000, 0xff00000000000000, 0xf600000000000000, 0xfc00000000000000, 0xf500000000000000, 0xfd00000000000000, 0x6e00000000000000, 0x6400000000000000, 0x6b00000000000000, 0x6200000000000000, 0x6300000000000000, 0x6800000000000000, 0x6000000000000000, 0x6900000000000000, 0x6100000000000000, 0x6a00000000000000, 0x6700000000000000, 0x6f00000000000000, 0x6600000000000000, 0x6c00000000000000, 0x6500000000000000, 0x6d00000000000000, 0xce00000000000000, 0xc400000000000000, 0xcb00000000000000, 0xc200000000000000, 0xc300000000000000, 0xc800000000000000, 0xc000000000000000, 0xc900000000000000, 0xc100000000000000, 0xca00000000000000, 0xc700000000000000, 0xcf00000000000000, 0xc600000000000000, 0xcc00000000000000, 0xc500000000000000, 0xcd00000000000000, 0x5e00000000000000, 0x5400000000000000, 0x5b00000000000000, 0x5200000000000000, 0x5300000000000000, 0x5800000000000000, 0x5000000000000000, 0x5900000000000000, 0x5100000000000000, 0x5a00000000000000, 0x5700000000000000, 0x5f00000000000000, 0x5600000000000000, 0x5c00000000000000, 0x5500000000000000, 0x5d00000000000000, 0xde00000000000000, 0xd400000000000000, 0xdb00000000000000, 0xd200000000000000, 0xd300000000000000, 0xd800000000000000, 0xd000000000000000, 0xd900000000000000, 0xd100000000000000, 0xda00000000000000, 0xd700000000000000, 0xdf00000000000000, 0xd600000000000000, 0xdc00000000000000, 0xd500000000000000, 0xdd00000000000000};

unsigned long long Tcon80_Piccolo[25] = {0x3d2900001c070000, 0x3e2500001a1f0000, 0x3f21000018170000, 0x383d0000162f0000, 0x3939000014270000, 0x3a350000123f0000, 0x3b31000010370000, 0x340d00000e4f0000, 0x350900000c470000, 0x360500000a5f0000, 0x3701000008570000, 0x301d0000066f0000, 0x3119000004670000, 0x32150000027f0000, 0x3311000000770000, 0x2c6d00003e8f0000, 0x2d6900003c870000, 0x2e6500003a9f0000, 0x2f61000038970000, 0x287d000036af0000, 0x2979000034a70000, 0x2a75000032bf0000, 0x2b71000030b70000, 0x244d00002ecf0000, 0x254900002cc70000};

unsigned long long Tcon128_Piccolo[31] = {0x8aad0000456d0000, 0x89a1000043750000, 0x88a50000417d0000, 0x8fb900004f450000, 0x8ebd00004d4d0000, 0x8db100004b550000, 0x8cb50000495d0000, 0x8389000057250000, 0x828d0000552d0000, 0x8181000053350000, 0x80850000513d0000, 0x879900005f050000, 0x869d00005d0d0000, 0x859100005b150000, 0x84950000591d0000, 0x9be9000067e50000, 0x9aed000065ed0000, 0x99e1000063f50000, 0x98e5000061fd0000, 0x9ff900006fc50000, 0x9efd00006dcd0000, 0x9df100006bd50000, 0x9cf5000069dd0000, 0x93c9000077a50000, 0x92cd000075ad0000, 0x91c1000073b50000, 0x90c5000071bd0000, 0x97d900007f850000, 0x96dd00007d8d0000, 0x95d100007b950000, 0x94d50000799d0000};


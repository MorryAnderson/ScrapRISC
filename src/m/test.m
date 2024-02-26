%% 文件路径
bp_folder_path = "../../outputs/temp";

iromfile = "../../_Logisim/ROM/debug.irom";
dromfile = "../../_Logisim/ROM/gol_Glider.drom";

netfile_irom = "../../outputs/temp/M3_instr_rom.net";
netfile_drom = "../../outputs/temp/M3_data_rom.net";
netfile_data_ram = "../../outputs/temp/M3_data_ram.net";
netfile_regfile  = "../../outputs/temp/M3_reg_file.net";
netfile_bht      = "../../outputs/temp/M3_bht.net";
netfile_btb      = "../../outputs/temp/M3_btb.net";
netfile_screen   = "../../outputs/temp/M3_screen.net";

%% 参数
DATA_WIDTH = 32;
ROM_DEPTH_INSTR = 128;
ROM_DEPTH_DATA = 64;
SHRINK = 1;

%%
DUAL_RD = 1;  % dual read port
BYTE_EN = 1;  % byte enable, only valid when WIDTH % 8 == 0
REG0 = 1;     % reg[0] === 0 (for regfile)

%% Instr ROM
GenNetRomPlane(iromfile,netfile_irom, ROM_DEPTH_INSTR);
Net2Blueprint(netfile_irom, bp_folder_path);

%% Data ROM
GenNetRomPlane(dromfile,netfile_drom, ROM_DEPTH_DATA);
Net2Blueprint(netfile_drom, bp_folder_path);

%% RAMs
GenNetRam(netfile_data_ram,32,64,~DUAL_RD,BYTE_EN,~REG0);
GenNetRam(netfile_regfile,32,16,DUAL_RD,~BYTE_EN,REG0);
GenNetRam(netfile_btb,32,16,DUAL_RD,~BYTE_EN,~REG0);
GenNetRam(netfile_bht,2,16,DUAL_RD,~BYTE_EN,~REG0);

Net2Blueprint(netfile_data_ram, bp_folder_path);
Net2Blueprint(netfile_regfile, bp_folder_path);
Net2Blueprint(netfile_btb, bp_folder_path);
Net2Blueprint(netfile_bht, bp_folder_path);

%% Screen
GenNetScreen(netfile_screen);
Net2Blueprint(netfile_screen, bp_folder_path);

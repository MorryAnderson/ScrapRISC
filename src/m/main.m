%% 文件路径
bp_folder_path = "../../outputs/temp";

iromfile = "../../_Logisim/ROM/debug.irom";
dromfile = "../../_Logisim/ROM/gol_Glider.drom";

netfile_irom = "../../outputs/temp/M3_instr_rom.net";
netfile_drom = "../../outputs/temp/M3_data_rom.net";


%% 参数
DATA_WIDTH = 32;
ROM_DEPTH_INSTR = 128;
ROM_DEPTH_DATA = 64;


%% Instr ROM
GenNetRomPlane(iromfile,netfile_irom, ROM_DEPTH_INSTR);
Net2Blueprint(netfile_irom, bp_folder_path);

%% Data ROM
GenNetRomPlane(dromfile,netfile_drom, ROM_DEPTH_DATA);
Net2Blueprint(netfile_drom, bp_folder_path);

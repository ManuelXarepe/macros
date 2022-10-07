
#include <iostream>
#include <string>
#include <unistd.h>

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t  unpack;
    EXT_STR_h101_RPC rpc;
    EXT_STR_h101_WRMASTER_t wrm;
    EXT_STR_h101_TPAT_t unpacktpat;
    EXT_STR_h101_LOS_t los;
    EXT_STR_h101_WRMASTER_t wrmaster;
    EXT_STR_h101_WRLOS_t wrlos;

} EXT_STR_h101;



 void rpc_online(){

   //gStyle->SetOptStat(0);
   FairLogger::GetLogger()->SetLogScreenLevel("INFO");
   FairLogger::GetLogger()->SetColoredLog(true);
   // FairLogger::GetLogger()->SetLogVerbosityLevel("VERYHIGH");


   TStopwatch timer;

   auto t = std::time(nullptr);
   auto tm = *std::localtime(&t);
   std::ostringstream oss;
   oss << std::put_time(&tm, "%Y%m%d_%H%M%S");

   const Int_t nev = -1; // number of events to read, -1 - until CTRL+C


//   TString filename = "--stream=lxlanddaq01:9000 --allow-errors --input-buffer=135Mi";
//   TString filename = "/lustre/r3b/mxarepe/sweep_lmd/main* --allow-errors --input-buffer=135Mi";
   TString filename = "/lustre/r3b/mxarepe/stitched_sweep/main0165_000*_TS.lmd --allow-errors --input-buffer=135Mi";
//   TString filename = "/lustre/r3b/mxarepe/lmd_stitched_data/main_TS_0127_0130.lmd --allow-errors --input-buffer=135Mi";
   // Online server configuration --------------------------
   Int_t refresh = 1; // Refresh rate for online histograms
   Int_t port = 6766; // Port number for the online visualization, example lxgXXXX:8887


   // UCESB configuration ----------------------------------
   TString ntuple_options = "RAW";
   TString ucesb_dir = getenv("UCESB_DIR");
   TString upexps_dir = ucesb_dir + "../upexps";
   TString outputpath = "/lustre/land/202205_s522/rootfiles/";
     TString outputFileName = "/u/land/mxarepe/unpkd_data/analysis/rootfiles/main0165_TS.root";
 //  TString outputFileName = "/lustre/r3b/mxarepe/unpacked_data/main_TS_0127_0130_with_TOFD.root";
 //  TString outputFileName = "teste.root";

   TString ucesb_path;

   ucesb_path ="/u/land/fake_cvmfs/9.13/upexps/202205_s522/202205_s522 --allow-errors --input-buffer=135Mi";
   //ucesb_path ="/u/land/fake_cvmfs/9.13/upexps/202205_s522/202205_s522 --allow-errors --input-buffer=135Mi";

   ucesb_path.ReplaceAll("//", "/");

   // Load ucesb structure ---------------------------------
   EXT_STR_h101 ucesb_struct;

   // Create online run ------------------------------------
   FairRunOnline* run = new FairRunOnline();
   R3BEventHeader* EvntHeader = new R3BEventHeader();
   run->SetEventHeader(EvntHeader);
   run->SetRunId(1);
   run->ActivateHttpServer(refresh, port);

   run->SetSink(new FairRootFileSink(outputFileName));
   
   TString calFile = "RpcPosHitcalPar_test0001.root";
   //TString calFile = "RpcPosHitcalPar_self0001.root";

    TString loscalfilename = "/u/land/r3broot/202205_s522/R3BParams_S522/los/loscalpar_v3.root";
    loscalfilename.ReplaceAll("//", "/");


    /* Declare an object to store mapping parameters */
    R3BTCalPar *rpcTcalPar;
    R3BRpcTotCalPar *rpcTotCalPar;
    R3BRpcHitPar * rpcHitPar;
    R3BTCalPar *losTCalPar;

    FairParRootFileIo *inputRoot = new FairParRootFileIo(kTRUE);


              FairRuntimeDb* rtdb1 = run->GetRuntimeDb();
              Bool_t kParameterMerged = kTRUE;
              FairParRootFileIo* parOut1 = new FairParRootFileIo(kParameterMerged);
              TList *parList = new TList();

              parList->Add(new TObjString(loscalfilename));//This is a standin. File not existent right now.
              parList->Add(new TObjString(calFile));
              parOut1->open(parList);

              rtdb1->setFirstInput(parOut1);
              rtdb1->print();

              rtdb1->addRun(1);
              rtdb1->getContainer("LosTCalPar");
              rtdb1->setInputVersion(1, (char*)"LosTCalPar", 1, 1);

              rtdb1->addRun(1);
              rtdb1->getContainer("RpcTCalPar");
              rtdb1->setInputVersion(1, (char*)"RpcTCalPar", 1, 1);


              rtdb1->addRun(1);
              rtdb1->getContainer("RpcTotCalPar");
              rtdb1->setInputVersion(1, (char*)"RpcTotCalPar", 1, 1);

              rtdb1->addRun(1);
              rtdb1->getContainer("RpcHitPar");
              rtdb1->setInputVersion(1, (char*)"RpcHitPar", 1, 1);




   /* Just a dummy RunID, it is not relevant in this case */
   rtdb1->initContainers(1);


   // Create source using ucesb for input ------------------
   R3BUcesbSource* source =
       new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
   source->SetMaxEvents(nev);

   // Definition of reader ---------------------------------
   R3BUnpackReader* unpackreader =
       new R3BUnpackReader((EXT_STR_h101_unpack*)&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack));

   R3BWhiterabbitMasterReader* unpackWRM =
       new R3BWhiterabbitMasterReader((EXT_STR_h101_WRMASTER*)&ucesb_struct.wrm, offsetof(EXT_STR_h101, wrm), 0x1000);


   R3BRpcReader* unpackRPC =
       new R3BRpcReader((EXT_STR_h101_RPC*)&ucesb_struct.rpc, offsetof(EXT_STR_h101,rpc));

   R3BLosReader *unpacklos;
   R3BWhiterabbitLosReader *unpackWRlos;

   unpackWRlos = new R3BWhiterabbitLosReader((EXT_STR_h101_WRLOS *)&ucesb_struct.wrlos,offsetof(EXT_STR_h101, wrlos), 0x1100); // FIXME
   unpackWRlos->SetOnline(kFALSE);
   source->AddReader(unpackWRlos);

   unpacklos = new R3BLosReader((EXT_STR_h101_LOS_t *)&ucesb_struct.los,offsetof(EXT_STR_h101, los));
   unpacklos->SetOnline(kFALSE);
   source->AddReader(unpacklos);

   source->AddReader(unpackreader);
   unpackWRM->SetOnline(kFALSE);
   source->AddReader(unpackWRM);

   source->AddReader(unpackRPC);
   unpackRPC->SetOnline(kFALSE);

   source->AddReader(new R3BTrloiiTpatReader(&ucesb_struct.unpacktpat, offsetof(EXT_STR_h101, unpacktpat)));

   run->SetSource(source);

   R3BLosMapped2Cal *losMapped2Cal =
   new R3BLosMapped2Cal("R3BLosMapped2Cal", 1);

   losMapped2Cal->SetNofModules(1, 8);
   losMapped2Cal->SetTrigger(-1);
   losMapped2Cal->SetOnline(kFALSE);
   run->AddTask(losMapped2Cal);


   auto losstart = new R3BLosProvideTStart();
   run->AddTask(losstart);

   R3BRpcMapped2PreCal* RpcMapped2PreCal=new R3BRpcMapped2PreCal();
   RpcMapped2PreCal->SetFpgaCorrelationFile("LUT.txt");
   run->AddTask(RpcMapped2PreCal);

   R3BRpcPreCal2Cal* RpcPreCal2Cal = new R3BRpcPreCal2Cal();
   run->AddTask(RpcPreCal2Cal);

   R3BRpcCal2Hit *cal2Hit = new R3BRpcCal2Hit();
   run->AddTask(cal2Hit);

   /*R3BRpcOnlineSpectra * rpcOnline = new R3BRpcOnlineSpectra();
   rpcOnline->SetTofAxisRange(300,-80,-50); // Bins, Left Limit, Right Limit
   rpcOnline->SetRpcTimeAxisRange(1000,-1000,1000);
   rpcOnline->SetRpcToTAxisRange(600,-400,400);

   run->AddTask(rpcOnline);*/

   timer.Start();

   // Initialize -------------------------------------------
   run->Init();

   // Informations about portnumber and main data stream.
   std::cout << "\n\n" << std::endl;
   std::cout << "Data stream is: " << filename << std::endl;
   std::cout << "Portnumber for Rpc online is: " << port << std::endl;
   std::cout << "\n\n" << std::endl;

   // Run --------------------------------------------------
   run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);
   // -----   Finish   -------------------------------------
   std::cout << std::endl << std::endl;


   timer.Stop();
   Double_t rtime = timer.RealTime();
   Double_t ctime = timer.CpuTime();

//   std::cout << "Output file is " << outputFileName << std::endl;
   std::cout << "Real time " << rtime << " s, CPU time " << ctime << "s" << std::endl << std::endl;
   Float_t cpuUsage = ctime / rtime;
   std::cout << "CPU used: " << cpuUsage << std::endl;
   std::cout << "Macro finished successfully." << std::endl;
   std::cout << std::endl;
   gApplication->Terminate();



}

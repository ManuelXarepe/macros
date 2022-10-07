/**
 **  Macro to run the offline for all the detectors simultaneously
 **
 **  This macro generates a root file with all the data at mapped level using
 **  a lmd file as input
 **
 **  Author: Jose Luis <joseluis.rodriguez.sanchez@usc.es>
 **  @since March 5th, 2022
 **
 **
 **  const Int_t nev = -1; number of events to read, -1 - until CTRL+C
 **  Select experiment ID: 522
 **
 **  After defining the input file (filename), execute the macro
 **  1) if all the parameters are right by default
 **     root -l unpack_data.C
 **  2) if one wants to select a RunId, for instance 'RunId = 173'
 **     root -l 'unpack_data.C(173)'
 **  3) if one wants to select a RunId and max number of events,
 **     for instance 'RunId = 173' and 'nev = 200'
 **     root -l 'unpack_data.C(173,200)'
 **  4) if one wants to select a RunId, max number of events and ExpId,
 **     for instance 'RunId = 173', 'nev = 200', and 'ExpId = 522'
 **     root -l 'unpack_data.C(173,200,522)'
 **
 **/

typedef struct EXT_STR_h101_t {
  EXT_STR_h101_unpack_t unpack;
  EXT_STR_h101_TPAT_t tpat;

  EXT_STR_h101_SCI2_onion_t s2;
  EXT_STR_h101_FOOT_onion_t foot;
  EXT_STR_h101_CALIFA_t califa;

  EXT_STR_h101_SOFMWPC_onion_t mwpc;
  EXT_STR_h101_LOS_onion_t los;
  EXT_STR_h101_MUSLI_onion_t musli;

  EXT_STR_h101_FIBEO_onion_t fib30; // Fib 30
  EXT_STR_h101_FIBEI_onion_t fib31; // Fib 31
  EXT_STR_h101_FIBEZ_onion_t fib32; // Fib 32
  EXT_STR_h101_FIBEE_onion_t fib33; // Fib 33
  EXT_STR_h101_TOFD_onion_t tofd;
  EXT_STR_h101_RPC_onion_t rpc;
  EXT_STR_h101_raw_nnp_tamex_onion_t neuland;

  EXT_STR_h101_WRMASTER_t wrmaster;
  EXT_STR_h101_WRNEULAND_t wrneuland;
  EXT_STR_h101_WRS2_t wrs2;
  EXT_STR_h101_WRFOOT_onion_t wrfoot;
} EXT_STR_h101;

void unpack_all(const Int_t fRunId = 70, const Int_t nev = -1, const Int_t fExpId = 522) 
{
  TString cRunId = Form("%04d", fRunId);
  TString cExpId = Form("%03d", fExpId);

  FairLogger::GetLogger()->SetLogScreenLevel("INFO");
  FairLogger::GetLogger()->SetColoredLog(true);

  TStopwatch timer;
TString dir = "/u/land/r3broot/202205_s522/r3broot";
//TString dir = gSystem->Getenv("VMCWORKDIR");
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y%m%d_%H%M%S");

  // file names and paths -------------------------------------------------------
  TString ntuple_options = "RAW";
  //TString ntuple_options = "RAW,time-stitch=7000";
  TString ucesb_dir = getenv("UCESB_DIR");
  TString filename, outputFilename, upexps_dir, ucesb_path, pardir;
  std::vector <UInt_t> WRsFoot;

  if (fExpId == 522) {
    // Input file
    // filename = "/lustre/r3b/mxarepe/stitched_sweep/main0167_000* --allow-errors --input-buffer=135Mi";
    filename = "/lustre/r3b/mxarepe/lmd_stitched_data/main_TS_0127_0130.lmd --allow-errors --input-buffer=135Mi";
    //filename = "/lustre/r3b/202205_s509/lmd/main0159_0002.lmd";

    // TString outputpath = "/u/land/r3broot/202205_s522/TOFD/git/R3BParams_S522/macros/exp/unpack/";
    // outputFilename = outputpath + "s"+cExpId+"_map_data_offline_" + oss.str() + ".root";
    outputFilename = "/lustre/r3b/mxarepe/unpacked_data/main_TS_0127_0130_with_TOFD.root";

    upexps_dir = ucesb_dir + "/../upexps";
    pardir = dir + "/../R3BParams_S522/";
    //ucesb_path = upexps_dir + "/202205_s522/202205_s522 --allow-errors --input-buffer=150Mi";
    ucesb_path ="/u/land/fake_cvmfs/9.13/upexps/202205_s522/202205_s522 --allow-errors --input-buffer=135Mi";
    ucesb_path.ReplaceAll("//", "/");
/*    WRsFoot.resize(11);
    // FIXME
    WRsFoot[ 1]=0x0400;
    WRsFoot[ 3]=0x0200;
    WRsFoot[ 6]=0x0500;
    WRsFoot[ 8]=0x0700;
    WRsFoot[ 9]=0x0800;
    WRsFoot[10]=0x0600; */ 
  } else {
    std::cout << "Experiment was not selected" << std::endl;
    gApplication->Terminate();
  }

  // Setup: Selection of detectors ----------------------------------------------
  // --- S2 -----------------------------------------------------
  Bool_t fFrsSci = false; // Start: Plastic scintillators at FRS
  // --- R3B standard -------------------------------------------
  Bool_t fMwpc0 = false;   // MWPC0 for tracking at entrance of Cave-C
  Bool_t fLos = true;     // Los scintillator for R3B experiments
  Bool_t fMusli = false;   // Music-light: Ionization chamber for charge-Z of fragments
  Bool_t fMwpc1 = false;   // MWPC1 for tracking of fragments before target
  Bool_t fFoot = false;   // Foot tracking detectors
  Bool_t fCalifa = false;  // Califa calorimeter
  Bool_t fFib30 = false;   // Fiber 30 for fragment tracking behind GLAD
  Bool_t fFib31 = false;   // Fiber 31 for fragment tracking behind GLAD
  Bool_t fFib32 = false;   // Fiber 32 for fragment tracking behind GLAD
  Bool_t fFib33 = false;   // Fiber 33 for fragment tracking behind GLAD
  Bool_t fRpc = true;     // RPC for protons behind GLAD
  Bool_t fTofD = true;    // ToF-Wall for time-of-flight of fragments behind GLAD
  Bool_t fNeuland = false; // NeuLAND for neutrons behind GLAD

  TString calfilename =  "/u/land/r3broot/202205_s522/R3BParams_S522/parameters/CalibPar.par";
  calfilename.ReplaceAll("//", "/");
  TString loscalfilename =  "/u/land/r3broot/202205_s522/R3BParams_S522/los/loscalpar_v3.root";
  loscalfilename.ReplaceAll("//", "/");
  TString footcalfilename = pardir + "xxx.par";
  footcalfilename.ReplaceAll("//", "/");

  // Create source using ucesb for input ----------------------------------------
  EXT_STR_h101 ucesb_struct;

  // Create online run ----------------------------------------------------------
  auto run = new FairRunOnline();
  auto EvntHeader = new R3BEventHeader();
  run->SetEventHeader(EvntHeader);
  run->SetRunId(fRunId);
  run->SetSink(new FairRootFileSink(outputFilename));

  auto source = new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct,
                                                                   sizeof(ucesb_struct));
  source->SetMaxEvents(nev);
  // source->SetInputFileName("setup_runid.par");

  // Add readers ----------------------------------------------------------------
  source->AddReader(new R3BUnpackReader(&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack)));

  source->AddReader(new R3BTrloiiTpatReader(&ucesb_struct.tpat, offsetof(EXT_STR_h101, tpat)));

  auto unpackWRMaster = new R3BWhiterabbitMasterReader((EXT_STR_h101_WRMASTER *)&ucesb_struct.wrmaster,offsetof(EXT_STR_h101, wrmaster), 0x1000);
  source->AddReader(unpackWRMaster);

  if (fFrsSci) {
    source->AddReader(new R3BWhiterabbitS2Reader((EXT_STR_h101_WRS2*)&ucesb_struct.wrs2,
                                                           offsetof(EXT_STR_h101, wrs2), 0x200));
    source->AddReader(new R3BSci2Reader((EXT_STR_h101_SCI2*)&ucesb_struct.s2, offsetof(EXT_STR_h101_t, s2)));
 

	  auto unpacks2 = new R3BSci2Reader((EXT_STR_h101_SCI2*)&ucesb_struct.s2,offsetof(EXT_STR_h101_t, s2));
//	  unpacks2->SetOnline(NOTstoremappeddata);
	  source->AddReader(unpacks2);
  }

  if (fLos){
    source->AddReader(new R3BLosReader((EXT_STR_h101_LOS*)&ucesb_struct.los, 
                                                   offsetof(EXT_STR_h101, los)));
  }

  if (fFoot) {
    source->AddReader(new R3BFootSiReader((EXT_STR_h101_FOOT_onion*)&ucesb_struct.foot, offsetof(EXT_STR_h101, foot)));
    source->AddReader(new R3BWhiterabbitFootReader((EXT_STR_h101_WRFOOT_onion*)&ucesb_struct.wrfoot,
                                                       offsetof(EXT_STR_h101, wrfoot), WRsFoot));
  }

  if (fCalifa) {
    source->AddReader(
        new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa,
                                                offsetof(EXT_STR_h101, califa)));
  }

  if (fMwpc0 || fMwpc1) {
    auto mwpcreader = new R3BMwpcReader((EXT_STR_h101_SOFMWPC*)&ucesb_struct.mwpc,
                                                      offsetof(EXT_STR_h101, mwpc));
    mwpcreader->SetMaxNbDet(2);
    source->AddReader(mwpcreader);
  }

  if (fMusli) {
    source->AddReader(new R3BMusliReader((EXT_STR_h101_MUSLI*)&ucesb_struct.musli,
                                                           offsetof(EXT_STR_h101, musli)));
  }

  if (fFib30) {
    source->AddReader(new R3BFiberReader("Fi30",512,(EXT_STR_h101_FIBEO_onion*)&ucesb_struct.fib30,
                                                                      offsetof(EXT_STR_h101_t, fib30)));
  }
  
  if (fFib31) {
    source->AddReader(new R3BFiberReader("Fi31",512,(EXT_STR_h101_FIBEI_onion*)&ucesb_struct.fib31,
                                                                      offsetof(EXT_STR_h101_t, fib31)));
  }
  
  if (fFib32) {
    source->AddReader(new R3BFiberReader("Fi32",512,(EXT_STR_h101_FIBEZ_onion*)&ucesb_struct.fib32,
                                                                      offsetof(EXT_STR_h101_t, fib32)));
  }
  
  if (fFib33) {
    source->AddReader(new R3BFiberReader("Fi33",512,(EXT_STR_h101_FIBEE_onion*)&ucesb_struct.fib33,
                                                                      offsetof(EXT_STR_h101_t, fib33)));
  }

  if (fTofD) {
    source->AddReader(new R3BTofdReader((EXT_STR_h101_TOFD_onion*)&ucesb_struct.tofd,
                                                      offsetof(EXT_STR_h101, tofd)));
  }
  
  if (fRpc){
	  source->AddReader(new R3BRpcReader((EXT_STR_h101_RPC*)&ucesb_struct.rpc,
				  offsetof(EXT_STR_h101, rpc)));
  }

  if (fNeuland) {
	  source->AddReader(new R3BNeulandTamexReader((EXT_STR_h101_raw_nnp_tamex_onion*)&ucesb_struct.neuland,
				  offsetof(EXT_STR_h101, neuland)));
	  source->AddReader(new R3BWhiterabbitNeulandReader((EXT_STR_h101_WRNEULAND*)&ucesb_struct.wrneuland,
				  offsetof(EXT_STR_h101, wrneuland), 0x900));
  }

  run->SetSource(source);
  FairRuntimeDb *rtdb = run->GetRuntimeDb();
  Bool_t kParameterMerged = kFALSE;
  auto parIo1 = new FairParRootFileIo(kParameterMerged);
  TList *parList1 = new TList();
  if (fFrsSci)
	  parList1->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/sci2/sci2calpar_v2.root"));
  if (fLos)
	  parList1->Add(new TObjString(loscalfilename));
  if (fTofD) 
	  parList1->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/tofd/tofdhitpar_v4.root"));
  if (fRpc)
          parList1->Add(new TObjString("RpcPosHitcalPar_test0001.root"));
  if (fFib30 || fFib31 || fFib32 || fFib33)
	  parList1->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/fibers/fiberhitpar_v2.root"));

  if (fFrsSci || fLos || fTofD || fFib30 || fFib31 || fFib32 || fFib33 || fRpc) 
  {
	  parIo1->open(parList1, "in");
	  rtdb->setFirstInput(parIo1);
  }
  rtdb->print();
  auto parIo2 = new FairParAsciiFileIo();
  TList *parList2 = new TList();
  parList2->Add(new TObjString(calfilename));
  parList2->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/tofd/tofdmappingpar_v1.par"));
  parList2->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/fibers/fibermappingpar_v1.par"));
  parList2->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/califa/califamappingparams_v1.par"));
  parList2->Add(new TObjString("/u/land/mxarepe/Clone_R3BRoot/R3BParams_S522/califa/califacalibparams_v2.par"));
  parIo2->open(parList2, "in");
  if (fFrsSci || fLos || fTofD || fFib30 || fFib31 || fFib32 || fFib33 || fRpc) 
	  rtdb->setSecondInput(parIo2);
  else
	  rtdb->setFirstInput(parIo2);

  if (fFrsSci || fLos || fTofD || fFib30 || fFib31 || fFib32 || fFib33 || fRpc){
	  rtdb->addRun(fRunId);
  }
  if (fFrsSci) {
	  rtdb->getContainer("Sci2TCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"Sci2TCalPar", 1, 1);
  }
  if (fLos) {
	  rtdb->getContainer("LosTCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"LosTCalPar", 1, 1);
  }
  if (fFib30){
	  rtdb->getContainer("Fi30MAPMTTCalPar");
	  rtdb->setInputVersion(fRunId, (char*)"Fi30MAPMTTCalPar", 1, 1);
	  rtdb->getContainer("Fi30MAPMTTrigTCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi30MAPMTTrigTCalPar", 1, 1);
	  rtdb->getContainer("Fi30HitPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi30HitPar", 1, 1);
  }
  if (fFib31){
	  rtdb->getContainer("Fi31MAPMTTCalPar");
	  rtdb->setInputVersion(fRunId, (char*)"Fi31MAPMTTCalPar", 1, 1);
	  rtdb->getContainer("Fi31MAPMTTrigTCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi31MAPMTTrigTCalPar", 1, 1);
	  rtdb->getContainer("Fi31HitPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi31HitPar", 1, 1);
  }

  if (fFib32){
	  rtdb->getContainer("Fi32MAPMTTCalPar");
	  rtdb->setInputVersion(fRunId, (char*)"Fi32MAPMTTCalPar", 1, 1);
	  rtdb->getContainer("Fi32MAPMTTrigTCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi32MAPMTTrigTCalPar", 1, 1);
	  rtdb->getContainer("Fi32HitPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi32HitPar", 1, 1);
  }
  if (fFib33){
	  rtdb->getContainer("Fi33MAPMTTCalPar");
	  rtdb->setInputVersion(fRunId, (char*)"Fi33MAPMTTCalPar", 1, 1);
	  rtdb->getContainer("Fi33MAPMTTrigTCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi33MAPMTTrigTCalPar", 1, 1);
	  rtdb->getContainer("Fi33HitPar");
	  rtdb->setInputVersion(fRunId, (char *)"Fi33HitPar", 1, 1);
  }
  if (fTofD){
	  rtdb->getContainer("tofdMappingPar");
	  rtdb->setInputVersion(fRunId, (char *)"tofdMappingPar", 1, 1);
	  rtdb->getContainer("TofdTCalPar");
	  rtdb->setInputVersion(fRunId, (char *)"TofdTCalPar", 1, 1);
	  rtdb->getContainer("tofdHitPar");
	  rtdb->setInputVersion(fRunId, (char *)"tofdHitPar", 1, 1);
  } 
  if (fRpc){
          rtdb->getContainer("RpcTCalPar");
          rtdb->setInputVersion(fRunId, (char*)"RpcTCalPar", 1, 1);
          rtdb->getContainer("RpcTotCalPar");
          rtdb->setInputVersion(fRunId, (char*)"RpcTotCalPar", 1, 1);
          rtdb->getContainer("RpcHitPar");
          rtdb->setInputVersion(fRunId, (char*)"RpcHitPar", 1, 1);
  }
  if (fCalifa){
          rtdb->getContainer("califaCrystalCalPar");
          rtdb->setInputVersion(fRunId,(char*)"califaCrystalCalPar",1,1);
          rtdb->getContainer("califaMappingPar");
          rtdb->setInputVersion(fRunId,(char*)"califaMappingPar",1,1);
  }
  rtdb->print();
if (fFrsSci){
  auto s2Mapped2Tcal = new R3BSci2Mapped2Tcal();
  //s2Mapped2Tcal->SetOnline(NOTstorecaldata);
  run->AddTask(s2Mapped2Tcal);

  auto sci2Tcal2Hit = new R3BSci2Tcal2Hit();
  //sci2Tcal2Hit->SetOnline(NOTstorehitdata);
  run->AddTask(sci2Tcal2Hit);
}
if (fMwpc0) {
	auto MW0Map2Cal = new R3BMwpc0Mapped2Cal();
	//MW0Map2Cal->SetOnline(NOTstorecaldata);
	run->AddTask(MW0Map2Cal);

	auto MW0Cal2Hit = new R3BMwpc0Cal2Hit();
	//MW0Cal2Hit->SetOnline(NOTstorehitdata);
	run->AddTask(MW0Cal2Hit);
}

if (fLos) {
	auto losMapped2Cal = new R3BLosMapped2Cal();
	losMapped2Cal->SetNofModules(1, 8);
	losMapped2Cal->SetTrigger(1);
	//losMapped2Cal->SetOnline(NOTstorecaldata);
	run->AddTask(losMapped2Cal);

	auto losstart = new R3BLosProvideTStart();
	run->AddTask(losstart);

//	auto losCal2Hit = new R3BLosCal2Hit();
//	losCal2Hit->SetTrigger(1);
	//losCal2Hit->SetOnline(NOTstorehitdata);
//	run->AddTask(losCal2Hit);
}
if (fMwpc1) {
	auto MW1Map2Cal = new R3BMwpc1Mapped2Cal();
	//MW1Map2Cal->SetOnline(NOTstorecaldata);
	run->AddTask(MW1Map2Cal);

	auto MW1Cal2Hit = new R3BMwpc1Cal2Hit();
	//MW1Cal2Hit->SetOnline(NOTstorehitdata);
	run->AddTask(MW1Cal2Hit);
}

if (fFib30){
	auto map2calFi30 = new R3BFiberMAPMTMapped2Cal("Fi30");
	//map2calFi30->SetOnline(NOTstorecaldata);
	run->AddTask(map2calFi30);
	auto cal2hitFi30 = new R3BFiberMAPMTCal2Hit("Fi30", 1, R3BFiberMAPMTCal2Hit::HORIZONTAL);
	//cal2hitFi30->SetOnline(NOTstorehitdata);
	run->AddTask(cal2hitFi30);
}


if (fFib31){
	auto map2calFi31 = new R3BFiberMAPMTMapped2Cal("Fi31");
	//map2calFi31->SetOnline(NOTstorecaldata);
	run->AddTask(map2calFi31);
	auto cal2hitFi31 = new R3BFiberMAPMTCal2Hit("Fi31", 1, R3BFiberMAPMTCal2Hit::VERTICAL);
	//cal2hitFi31->SetOnline(NOTstorehitdata);
	run->AddTask(cal2hitFi31);
}

if (fFib32){
	auto map2calFi32 = new R3BFiberMAPMTMapped2Cal("Fi32");
	//map2calFi32->SetOnline(NOTstorecaldata);
	run->AddTask(map2calFi32);
	auto cal2hitFi32 = new R3BFiberMAPMTCal2Hit("Fi32", 1, R3BFiberMAPMTCal2Hit::VERTICAL);
	//cal2hitFi32->SetOnline(NOTstorehitdata);
	run->AddTask(cal2hitFi32);
}

if (fFib33){
	auto map2calFi33 = new R3BFiberMAPMTMapped2Cal("Fi33");
	//map2calFi33->SetOnline(NOTstorecaldata);
	run->AddTask(map2calFi33);
	auto cal2hitFi33 = new R3BFiberMAPMTCal2Hit("Fi33", 1, R3BFiberMAPMTCal2Hit::VERTICAL);
	//cal2hitFi33->SetOnline(NOTstorehitdata);
	run->AddTask(cal2hitFi33);
}

if (fTofD){
	auto tofdMapped2Cal = new R3BTofDMapped2Cal();
	//tofdMapped2Cal->SetOnline(NOTstorecaldata);
	run->AddTask(tofdMapped2Cal);
	auto tofdCal2Hit = new R3BTofDCal2Hit();
	//tofdCal2Hit->SetOnline(NOTstorehitdata);
	tofdCal2Hit->SetTofdQ(6);          // Set nuclear charge Z of main beam
	tofdCal2Hit->SetTrigger(1);
	tofdCal2Hit->SetTofdTotPos(true); // use y-position from ToT instead of Tdiff
	run->AddTask(tofdCal2Hit);
}
// RPC
if (fRpc) { 
        auto rpcMapped2PreCal=new R3BRpcMapped2PreCal();
        rpcMapped2PreCal->SetFpgaCorrelationFile("LUT.txt");
//        rpcMapped2PreCal->SetOnline(NOTstorecaldata);
        run->AddTask(rpcMapped2PreCal);

        auto rpcPreCal2Cal = new R3BRpcPreCal2Cal();
//        rpcPreCal2Cal->SetOnline(NOTstorecaldata);
        run->AddTask(rpcPreCal2Cal);

        auto rpcCal2Hit = new R3BRpcCal2Hit();
  //      rpcCal2Hit->SetOnline(NOTstorehitdata);
        run->AddTask(rpcCal2Hit);
}
if(fCalifa){
	R3BCalifaMapped2CrystalCal* Map2Cal = new R3BCalifaMapped2CrystalCal();
	//Map2Cal->SetOnline(NOTstorecaldata);
	run->AddTask(Map2Cal);

	R3BCalifaCrystalCal2Hit* Cal2Hit = new R3BCalifaCrystalCal2Hit();
	Cal2Hit->SetCrystalThreshold(300.);//100keV
	Cal2Hit->SetDRThreshold(12000.);//12MeV
	Cal2Hit->SelectGeometryVersion(2021);
	//Cal2Hit->SetOnline(NOTstorehitdata);
	run->AddTask(Cal2Hit);




}
// Initialize -----------------------------------------------------------------
timer.Start();
run->Init();

// Run ------------------------------------------------------------------------
run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

// Finish ---------------------------------------------------------------------
timer.Stop();
Double_t rtime = timer.RealTime() / 60.;
Double_t ctime = timer.CpuTime() / 60.;
std::cout << std::endl << std::endl;
std::cout << "Macro finished succesfully." << std::endl;
std::cout << "Output file is " << outputFilename << std::endl;
std::cout << "Real time " << rtime << " min, CPU time " << ctime << " min"
<< std::endl
<< std::endl;
gApplication->Terminate();
}

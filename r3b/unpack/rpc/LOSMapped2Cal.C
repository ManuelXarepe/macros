//#include "FairParRootFileIo.h"

struct EXT_STR_h101_t
{
        EXT_STR_h101_unpack_t unpack;
        EXT_STR_h101_TPAT_t TPAT;
        EXT_STR_h101_LOS_t los; 
        EXT_STR_h101_timestamp_master_t timestamp_master;
        EXT_STR_h101_WRLOS_t wrlos;
        EXT_STR_h101_SAMP_t samp;				
};

void LOSMapped2Cal(Int_t RunId = 0)
{
    TString runNumber=Form ("%04d", RunId);
   
	TStopwatch timer;
	timer.Start();

	//const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */
	const Int_t nev = 1e+6; /* number of events to read, -1 - until CTRL+C */
//        TString filename = "/u/land/mxarepe/test_pulser_26_04.lmd --allow-errors  --input-buffer=150Mi";
          TString filename = "/u/land/mxarepe/test_pulser_new_Trb_4_05.lmd --allow-errors  --input-buffer=150Mi";
//        TString filename = "/lustre/r3b/202202_para/lmd/main0048_0001.lmd --allow-errors  --input-buffer=150Mi";
    
    TString outputFileName ="losTCalTestPulser.root";
 
    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");

    TString ucesb_path;
	ucesb_path = "/u/land/fake_cvmfs/9.13/upexps/202204_tests/202204_rpc";
//        ucesb_path = "/u/land/mxarepe/upexps/202202_para/202202_para --input-buffer=135Mi --allow-errors";
	EXT_STR_h101 ucesb_struct;
	R3BUcesbSource* source = new R3BUcesbSource(filename, ntuple_options,
	    ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
	source->SetMaxEvents(nev);

    source->AddReader(new R3BUnpackReader(
        (EXT_STR_h101_unpack_t *)&ucesb_struct,
        offsetof(EXT_STR_h101, unpack)));
    source->AddReader( new R3BLosReader (
	   (EXT_STR_h101_LOS_t *)&ucesb_struct.los, 
	   offsetof(EXT_STR_h101, los)) );
    source->AddReader(new R3BTrloiiTpatReader(
           (EXT_STR_h101_TPAT_t *)&ucesb_struct.TPAT,
           offsetof(EXT_STR_h101, TPAT)));


  const Int_t refresh = 100;  /* refresh rate for saving */

  /* Create online run ------------------------------------ */
#define RUN_ONLINE
#define USE_HTTP_SERVER
#ifdef RUN_ONLINE
  FairRunOnline* run = new FairRunOnline(source);
  run->SetRunId(RunId);
#ifdef USE_HTTP_SERVER
  run->ActivateHttpServer(refresh);
#endif
#else
  /* Create analysis run ---------------------------------- */
  FairRunAna* run = new FairRunAna();
#endif

    run->SetOutputFile(outputFileName.Data());


	/* Runtime data base ------------------------------------ */
	FairRuntimeDb* rtdb = run->GetRuntimeDb();
	Bool_t kParameterMerged = kTRUE;
	TString parFileName    = "loscal_s522_Manuel.root";  // name of parameter file
	FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
	parOut->open(parFileName);
    rtdb->setFirstInput(parOut);
    rtdb->setOutput(parOut);


	/* Calibrate Los ---------------------------------------- */
    const Int_t updateRate = 1000;
    const Int_t minStats = 10000;        // minimum number of entries for TCAL calibration
    const Int_t trigger = 1;
    R3BLosMapped2CalPar* losCalibrator = new R3BLosMapped2CalPar("R3BLosMapped2CalPar", 1);
    losCalibrator->SetUpdateRate(updateRate);
    losCalibrator->SetMinStats(minStats);
    losCalibrator->SetTrigger(trigger);
    losCalibrator->SetNofModules(2, 8); // dets, bars(incl. master trigger)
    run->AddTask(losCalibrator);
	/* Calibrate Los - END */


	/* Initialize ------------------------------------------- */
	run->Init();
	FairLogger::GetLogger()->SetLogScreenLevel("INFO");
	//FairLogger::GetLogger()->SetLogScreenLevel("WARNING");
	//FairLogger::GetLogger()->SetLogScreenLevel("ERROR");
	/* ------------------------------------------------------ */


	/* Run -------------------------------------------------- */
	run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);
	if (rtdb->getCurrentRun()) cout << "have run" << endl;
	else cout << "have no run" << endl;
	rtdb->saveOutput();
	/* ------------------------------------------------------ */

	timer.Stop();
	Double_t rtime = timer.RealTime();
	Double_t ctime = timer.CpuTime();
	cout << endl << endl;
	cout << "Macro finished succesfully." << endl;
	cout << "Output file is " << outputFileName << endl;
	cout << "Real time " << rtime << " s, CPU time " << ctime << " s"
		     << endl << endl;
}


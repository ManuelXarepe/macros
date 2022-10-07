/*
 * In order to generate input for this, please go to $UCESB_DIR and run:
 *
 * Additional info:
 * To generate the header file used for the R3BUcesbSource (ext_h101.h), use:
 *
 * $UCESB_DIR/upexps/s438b/s438b --ntuple=UNPACK:TRIGGER,UNPACK:EVENTNO,RAW\
 *     STRUCT_HH,ext_h101.h
 *
 * Put this header file into the 'r3bsource' directory and recompile.
 *
 */

#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t  unpack;
    EXT_STR_h101_RPC rpc;
    EXT_STR_h101_WRMASTER_t wrm;
    EXT_STR_h101_TPAT_t unpacktpat;
    EXT_STR_h101_LOS_t los;
    EXT_STR_h101_WRMASTER_t wrmaster;
    EXT_STR_h101_WRLOS_t wrlos;
};

void RpcCal2HitPar(Int_t RunId=1)
{
    TString runNumber=Form ("%04d", RunId);
//    TString lmdNumber=Form ("%04d", lmdId);
    TStopwatch timer;
    timer.Start();

    Int_t trigger_value = 1;

    // Turn off automatic backtrace 
    gSystem->ResetSignals();


    const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */

    /* Create source using ucesb for input ------------------ */
   // TString filename = "/u/land/mxarepe/self_trigger_15_05_1.lmd";
   //TString filename = "/lustre/r3b/mxarepe/lmd_stitched_data/main_TS_0127_000*.lmd --allow-errors --input-buffer=135Mi";
   TString filename = "/lustre/r3b/mxarepe/stitched_sweep/main0164_0002_TS.lmd --allow-errors --input-buffer=135Mi";
//    TString filename = "main0090.lmd";
    Bool_t rpc_in  = true;

    TString outputFileName = "output.root";

    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");

    TString ucesb_path;
//      ucesb_path = "/u/land/mxarepe/upexps/202205_s522/202205_s522 --input-buffer=135Mi";
//    ucesb_path = "/u/land/mxarepe/upexps/202202_para/202202_para --input-buffer=135Mi --allow-errors";
    ucesb_path = "/u/land/fake_cvmfs/9.13/upexps/202205_s522/202205_s522 --input-buffer=135Mi --allow-errors";

        //--------- READERS ------------
    EXT_STR_h101_t ucesb_struct;
    R3BUcesbSource* source = new R3BUcesbSource(filename, ntuple_options,
        ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    source->AddReader( new R3BRpcReader (
           &ucesb_struct.rpc,
           offsetof(EXT_STR_h101, rpc)) );

    source->AddReader(new R3BTrloiiTpatReader(&ucesb_struct.unpacktpat, offsetof(EXT_STR_h101, unpacktpat)));


    /* Create analysis run ---------------------------------- */
    FairRunOnline* run = new FairRunOnline(source);
    R3BEventHeader* EvntHeader = new R3BEventHeader();
    run->SetEventHeader(EvntHeader);
    run->SetRunId(1);
    const Int_t refresh = 100;
    Int_t port = 6905;
    run->ActivateHttpServer(refresh, port);

    run->SetOutputFile(outputFileName.Data());


    /* Here goes your parameter file */
    TString calFile = "./RpcTotcalPar_sweep_0001_new.root";
//    TString calFile = "./RpcTotcalPar_0001.root";

    /* Declare an object to store mapping parameters */
    R3BTCalPar *rpcTcalPar;
    R3BRpcTotCalPar *rpcTotCalPar;

    FairParRootFileIo *inputRoot = new FairParRootFileIo(kTRUE);

    FairRuntimeDb *rtdb = FairRuntimeDb::instance();

    inputRoot->open(calFile, "in");

    rpcTcalPar = (R3BTCalPar *)rtdb->getContainer("RpcTCalPar");
    rpcTotCalPar = (R3BRpcTotCalPar *)rtdb->getContainer("RpcTotCalPar");

    rtdb->setFirstInput(inputRoot);


    /* Add analysis tasks ----------------------------------- */

    // Rpc
    R3BRpcMapped2PreCal* RpcMapped2PreCal=new R3BRpcMapped2PreCal();
    run->AddTask( RpcMapped2PreCal );

    RpcMapped2PreCal->SetFpgaCorrelationFile("/u/land/mxarepe/Clone_R3BRoot/rpc/calibration/LUT.txt");

    R3BRpcPreCal2Cal* RpcPreCal2Cal = new R3BRpcPreCal2Cal();
    run->AddTask(RpcPreCal2Cal);

    R3BRpcCal2HitPar* RpcCalibrator = new R3BRpcCal2HitPar("RpcHitPar", 1);
    run->AddTask(RpcCalibrator);

    TString parFileName = "./RpcPosHitcalPar_sweep"+runNumber+".root";  // name of parameter file
    //TString parFileName = "./RpcPosHitcalPar_self"+runNumber+".root";  // name of parameter file
    FairParRootFileIo* parOut = new FairParRootFileIo(kTRUE);
    parOut->open(parFileName, "out");   
    rtdb->setOutput(parOut);


    /* Initialize -------- ----------------------------------- */
    run->Init();
    // FairLogger::GetLogger()->SetLogScreenLevel("WARNING");
    // FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    FairLogger::GetLogger()->SetLogScreenLevel("ERROR");
    /* ------------------------------------------------------ */

    /* Run -------------------------------------------------- */
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);
    //run->Run(0,nev);
    //   if(nev < 0) run->Run();
    //      if(nev > 0) run->Run(0, nev);
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
                                                                   

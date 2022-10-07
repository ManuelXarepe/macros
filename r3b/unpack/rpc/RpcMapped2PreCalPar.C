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
        EXT_STR_h101_unpack_t unpack;
        EXT_STR_h101_RPC rpc;
};

void RpcMapped2PreCalPar(Int_t RunId=1)
{
    TString runNumber=Form ("%04d", RunId);
    TStopwatch timer;
    timer.Start();

    Int_t trigger_value = 1;

    // Turn off automatic backtrace 
    gSystem->ResetSignals();


    const Int_t nev = -1; /* number of events to read, -1 - until CTRL+C */

    /* Create source using ucesb for input ------------------ */
//   TString filename = "/lustre/r3b/mxarepe/lmd_stitched_data/main_TS_0127_0001.lmd --allow-errors --input-buffer=135Mi";
   TString filename = "/lustre/r3b/mxarepe/stitched_sweep/main0164_0002_TS.lmd --allow-errors --input-buffer=135Mi";
//   TString filename = "main0090.lmd --allow-errors --input-buffer=135Mi";

    Bool_t rpc_in  = true;

    TString outputFileName = "output.root";

    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");

    TString ucesb_path;
    //ucesb_path = "/u/land/mxarepe/upexps/202205_s522/202205_s522 --input-buffer=135Mi --allow-errors";
//    ucesb_path = "/u/land/fake_cvmfs/9.13/upexps/202204_tests/202204_rpc --input-buffer=135Mi --allow-errors";
    ucesb_path = "/u/land/fake_cvmfs/9.13/upexps/202205_s522/202205_s522 --input-buffer=135Mi --allow-errors";
    //ucesb_path = "/home/manuel/R3Broot_dev/upexps/202205_s522/202205_s522";

    //--------- READERS ------------
    EXT_STR_h101_t ucesb_struct;
    R3BUcesbSource* source = new R3BUcesbSource(filename, ntuple_options,
        ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    source->AddReader( new R3BRpcReader (
    &ucesb_struct.rpc,
    offsetof(EXT_STR_h101, rpc)) );

    /* Create analysis run ---------------------------------- */
    FairRunOnline* run = new FairRunOnline(source);
    run->SetRunId(1);
    const Int_t refresh = 100;
    Int_t port = 6905;
    run->ActivateHttpServer(refresh, port);

    run->SetOutputFile(outputFileName.Data());


    /* Runtime data base ------------------------------------ */
    FairRuntimeDb *rtdb = FairRuntimeDb::instance();
    TString parFileName = "./RpcTcalPar_sweep"+runNumber+".root";  // name of parameter file
//    TString parFileName = "./RpcTcalPar"+runNumber+".root";  // name of parameter file
//    TString parFileName = "./RpcTcalPar_self_"+runNumber+".root";  // name of parameter file

    FairParRootFileIo* parOut = new FairParRootFileIo(kTRUE);
    parOut->open(parFileName);

    //rtdb1->setFirstInput(parOut);
    rtdb->setOutput(parOut); 
    /* Add analysis tasks ----------------------------------- */

    // Rpc
    R3BRpcMapped2PreCalPar* RpcCalibrator=new R3BRpcMapped2PreCalPar("R3BRpcMapped2PreCalPar",1);
    run->AddTask( RpcCalibrator );

    /* Initialize -------- ----------------------------------- */
    run->Init();
    //      FairLogger::GetLogger()->SetLogScreenLevel("WARNING");
    // FairLogger::GetLogger()->SetLogScreenLevel("INFO");
    FairLogger::GetLogger()->SetLogScreenLevel("ERROR");
    /* ------------------------------------------------------ */


    /* Run -------------------------------------------------- */
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);
    // if(nev < 0) run->Run();
    // if(nev > 0) run->Run(0, nev);
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

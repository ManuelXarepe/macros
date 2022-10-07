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

void RpcCal2HitParNB(Int_t RunId=1)
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
    TString filename = "~/mxarepe/unpkd_data/r3b22038212351.lmd";

    Bool_t rpc_in  = true;

    TString outputFileName = "output.root";

    TString ntuple_options = "RAW";
    TString ucesb_dir = getenv("UCESB_DIR");

    TString ucesb_path;
    //ucesb_path = "/u/land/mxarepe/202205_s522/202205_s522";
    ucesb_path = "~/fake_cvmfs/9.13/upexps/202205_s522/202205_s522 --input-buffer=70Mi";

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


    /* Here goes your parameter file */
    TString calFile = "./RpcPosHitcalPar_0001.root";

    /* Declare an object to store mapping parameters */
    R3BTCalPar *rpcTcalPar;
    R3BRpcTotCalPar *rpcTotCalPar;
    R3BRpcHitPar *rpcHitPar;

    FairParRootFileIo *inputRoot = new FairParRootFileIo(kTRUE);

    FairRuntimeDb *rtdb = FairRuntimeDb::instance();

    inputRoot->open(calFile, "in");

    rpcTcalPar = (R3BTCalPar *)rtdb->getContainer("RpcTCalPar");
    rpcTotCalPar = (R3BRpcTotCalPar *)rtdb->getContainer("RpcTotCalPar");
    rpcHitPar = (R3BRpcHitPar *)rtdb->getContainer("RpcHitPar");

    rtdb->setFirstInput(inputRoot);


    /* Add analysis tasks ----------------------------------- */

    // Rpc
    R3BRpcMapped2PreCal* RpcMapped2PreCal=new R3BRpcMapped2PreCal();
    run->AddTask( RpcMapped2PreCal );

    RpcMapped2PreCal->SetFpgaCorrelationFile("/home/manuel/Clone_R3BRoot/rpc/calibration/LUT.txt");

    R3BRpcPreCal2Cal* RpcPreCal2Cal = new R3BRpcPreCal2Cal();
    run->AddTask(RpcPreCal2Cal);

    R3BRpcCal2HitPar* RpcCalibrator = new R3BRpcCal2HitPar("RpcHitPar", 1);
    run->AddTask(RpcCalibrator);

    RpcCalibrator->SetNBCalib();
    TString parFileName = "./RpcPosHitcalPar_NB_"+runNumber+".root";  // name of parameter file
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
                                                                   

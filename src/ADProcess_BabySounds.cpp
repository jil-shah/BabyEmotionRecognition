#include "../include/ADProcess_BabySounds.h"

std::vector<float> extract_mfcc(const char* filename){
    std::vector<float> mfcc_features(N_MFCC, 0.0);

    // Open WAV file
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Error opening audio file!\n";
        return mfcc_features;
    }

    // Initialize Aubio buffers
    fvec_t* input = new_fvec(FRAME_SIZE);
    cvec_t* fft_output = new_cvec(FRAME_SIZE);
    aubio_fft_t* fft = new_aubio_fft(FRAME_SIZE);
    aubio_mfcc_t* mfcc = new_aubio_mfcc(FRAME_SIZE, N_FILTERS, N_MFCC, SAMPLE_RATE);

    std::vector<float> mfcc_sum(N_MFCC, 0.0);
    int frame_count = 0;

    // Read audio and process frames
    while (sf_read_float(file, input->data, FRAME_SIZE) > 0) {
        // Apply pre-emphasis filter (to match Librosa)
        for (int i = FRAME_SIZE - 1; i > 0; i--) {
            input->data[i] -= 0.97 * input->data[i - 1];
        }

        // Compute FFT
        aubio_fft_do(fft, input, fft_output);

        // Compute MFCCs
        fvec_t* mfcc_out = new_fvec(N_MFCC);
        aubio_mfcc_do(mfcc, fft_output, mfcc_out);

        // Accumulate MFCCs for averaging
        for (int i = 0; i < N_MFCC; i++) {
            mfcc_sum[i] += mfcc_out->data[i];
        }

        del_fvec(mfcc_out);
        frame_count++;
    }

    // Compute the mean MFCC values
    if (frame_count > 0) {
        for (int i = 0; i < N_MFCC; i++) {
            mfcc_features[i] = mfcc_sum[i] / frame_count;
        }
    }

    // Cleanup
    sf_close(file);
    del_aubio_fft(fft);
    del_cvec(fft_output);
    del_aubio_mfcc(mfcc);
    del_fvec(input);

    return mfcc_features;
}

int pyPredict_Emotions(){
    
    std::cout << "\n\n-------------------------------------------------------------\n";

    Py_Initialize(); // Initialize Python interpreter

    if(!Py_IsInitialized()){
        std::cerr << "Failed to Initialize Python Interpreter" << std::endl;
        return -1;
    }

    // Run simple Python command
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(SITE_PACKAGES);
    PyRun_SimpleString(PYTHON_SCRIPT);

    // Import and call a function from a Python script
    PyObject *pName, *pModule, *pFunc, *pValue, *pArgs;
    
    pName = PyUnicode_DecodeFSDefault(PY_FILE);  // Name of script (without .py)
    pModule = PyImport_Import(pName);  // Import script.py
    Py_XDECREF(pName);

    if (pModule) {
        pFunc = PyObject_GetAttrString(pModule, "predictFromFile"); // Function name

        if (pFunc && PyCallable_Check(pFunc)) {

            pArgs = PyTuple_Pack(1,PyUnicode_FromString(MFCC_FILE));

            pValue = PyObject_CallObject(pFunc, pArgs);  // Call function
            Py_XDECREF(pArgs);

            if(pValue){
                std::cout << "-------------------------------------------------------------\n\n";
                std::cout << "Python Returned: " << PyUnicode_AsUTF8(pValue) << std::endl;
            }

        } else {
            PyErr_Print();
        }

        Py_XDECREF(pFunc);
        Py_XDECREF(pModule);
    } else {
        PyErr_Print();
    }

    Py_Finalize(); // Shutdown Python interpreter
    return 0;
}

int export_mfccFile(std::vector<float> mfcc_features, const char* txtFile){

    std::ofstream FILE(txtFile);

    if(!FILE){
        std::cerr << "Error: Could not open the file for writing\n";
        return 0; 
    }

    for(const float& value : mfcc_features){
        FILE << value << "\n";
    }

    FILE.close();
    std::cout << "MFCCs Features written to " << txtFile << "\n";

    return 0; 
}
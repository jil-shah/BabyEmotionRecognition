#include "../include/ADProcess_BabySounds.h"

std::vector<float> extract_mfcc(const char* filename){
    std::vector<float> mfcc_features(N_MFCC, 0.0);

    // Open WAV file
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Error opening audio file!\n";
        exit(1);
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

int export_mfccFile(std::vector<float> mfcc_features, const char* txtFile){

    std::ofstream FILE(txtFile); // open file pointer for writing

    if(!FILE){  // error detection
        std::cerr << "Error: Could not open the file for writing\n";
        return 0; 
    }

    for(const float& value : mfcc_features){  // write all contents of the vector to a txt file
        FILE << value << "\n";
    }

    FILE.close(); // close file 
    std::cout << "MFCCs Features written to " << txtFile << "\n"; 

    return 0; 
}

std::string readPredication(const char* filename){

    std::ifstream FILE(filename); // open file for reading

    if(!FILE){ // error detection
        std::cerr << "Error: Could not open file " << filename << std::endl; 
        return "1";
    }

    std::string line; 

    std::getline(FILE,line); // get the first line of the file
    FILE.close(); 

    // Delete the file after reading its content
    if (std::filesystem::remove(filename)) {
        std::cout << "File '" << filename << "' deleted successfully.\n";
    } else {
        std::cerr << "Error: Failed to delete file '" << filename << "'\n";
    }

    return line;  // return the predicted symptom from file 
}

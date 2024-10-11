#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

using namespace cv;
using namespace std;

int blockSize = 20;  // block size for block color detection
int maxBoxInFrame = 30;  // max boxes in a frame
double whiteThreshold = 0.0;  // white threshold for block color detection

void scaleframe(Mat& frame) {
    // scale frame to max width
    int max_width = 640;  // the video of bad apple is 640x480
    double scale = static_cast<double>(max_width) / frame.cols;
    resize(frame, frame, Size(), scale, scale);
}

vector<vector<int>> blockColor(Mat& frame, int blockSize) {
    // divide frame into blocks and determine if the block is white or black
    int rows = (frame.rows + blockSize - 1) / blockSize; 
    int cols = (frame.cols + blockSize - 1) / blockSize;
    vector<vector<int>> blocks(rows, vector<int>(cols, 0));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int startX = j * blockSize;
            int startY = i * blockSize;
            int width = min(blockSize, frame.cols - startX);
            int height = min(blockSize, frame.rows - startY);
            Mat block = frame(Rect(startX, startY, width, height));
            Scalar meanColor = mean(block);
            if (meanColor[0] > 128) {
                blocks[i][j] = 1;
            } else {
                blocks[i][j] = 0;
            }
        }
    }
    return blocks;
}

bool targetWhite(const vector<vector<int>>& blocks) {
    // determine if the target color is white or black
    int whiteBlocks = 0;
    for (const auto& row : blocks) {
        for (int block : row) {
            if (block == 1) {
                whiteBlocks++;
            }
        }
    }
    return whiteBlocks < blocks.size() * blocks[0].size() * whiteThreshold;
}

void mergeBoxesRow(vector<Rect>& boxes, int blockSize){
    // Merge boxes in the same row
    for (int i = 0; i < boxes.size(); i++) {
        for (int j = i + 1; j < boxes.size(); j++) {
            if (boxes[i].y == boxes[j].y && boxes[i].x + boxes[i].width == boxes[j].x) {
                boxes[i].width += blockSize;
                boxes.erase(boxes.begin() + j);
                j--;
            }
        }
    }
}

void mergeBoxesCol(vector<Rect>& boxes, int blockSize){
    // Merge boxes in the same column
    for (int i = 0; i < boxes.size(); i++) {
        for (int j = i + 1; j < boxes.size(); j++) {
            if (boxes[i].x == boxes[j].x && boxes[i].y + boxes[i].height == boxes[j].y && boxes[i].width == boxes[j].width) {
                boxes[i].height += blockSize;
                boxes.erase(boxes.begin() + j);
                j--;
            }
        }
    }
}

vector<Rect> targetBlocks2Boxes(const vector<vector<int>>& blocks, int blockSize) {
    // Convert target blocks to boxes
    vector<Rect> boxes;
    int target = targetWhite(blocks) ? 1 : 0;
    int rows = blocks.size();
    int cols = blocks[0].size();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (blocks[i][j] == target) {
                int startX = j;
                int startY = i;
                boxes.push_back(Rect(startX * blockSize, startY * blockSize, blockSize, blockSize));
            }
        }
    }
    mergeBoxesRow(boxes, blockSize);
    mergeBoxesCol(boxes, blockSize);
    return boxes;
}

void video2boxes(const string& videoPath, const string& outputPath){
    // Convert video to boxes
    VideoCapture cap(videoPath);
    if(!cap.isOpened()){
        cerr << "Error: Unable to open video file" << endl;
        return;
    }

    Mat frame;
    int frameCount = 0;
    int totalFrames = cap.get(CAP_PROP_FRAME_COUNT);

    ofstream file(outputPath);
    while(cap.read(frame)){
        scaleframe(frame);

        int boxesnum = 0;

        vector<Rect> boxes;
        int blockSizes = blockSize;

        do{
            vector<vector<int>> blocks = blockColor(frame, blockSizes);
            vector<Rect> box = targetBlocks2Boxes(blocks, blockSizes);
            boxes = box;
            boxesnum = box.size();
            blockSizes+=10;
        }while(boxesnum > maxBoxInFrame);
        
        for(const Rect& box : boxes){
            file << frameCount << " " << box.x << " " << box.y << " " << box.width << " " << box.height << endl;
        }

        frameCount++;

        // print progress
        cout << "Calculating Boxes " << frameCount << "/" << totalFrames << endl;
        cout.flush();
    }
}

void showBoxesinFrame(const string& videoPath, const string& boxesPath, const string& framePath){
    // debug use
    // The frames with boxes will be saved in framePath, which is set to "../frames"
    if(!filesystem::exists(videoPath)){
        cerr << "Error: Video file does not exist" << endl;
        return;
    }

    if(!filesystem::exists(boxesPath)){
        cerr << "Error: Boxes file does not exist" << endl;
        return;
    }

    if(!filesystem::exists(framePath)){
        filesystem::create_directory(framePath);
    }

    VideoCapture cap(videoPath);
    if(!cap.isOpened()){
        cerr << "Error: Unable to open video file" << endl;
        return;
    }

    Mat frame;
    int frameCount = 0;
    int totalFrames = cap.get(CAP_PROP_FRAME_COUNT);

    // read boxes from file "/doc/boxes.txt"
    ifstream file(boxesPath);
    vector<vector<Rect>> allBoxes(totalFrames);
    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int frameIndex, x, y, width, height;
        if (!(iss >> frameIndex >> x >> y >> width >> height)) {
            cerr << "Error: Invalid boxes file" << endl;
            return;
        }
        allBoxes[frameIndex].push_back(Rect(x, y, width, height));
    }

    while (cap.read(frame)) {
        scaleframe(frame);

        const vector<Rect>& boxes = allBoxes[frameCount];
        for (const Rect& box : boxes) {
            rectangle(frame, box, Scalar(0, 0, 255), 2);
        }

        imwrite(framePath + "/" + to_string(frameCount) + ".jpg", frame);

        frameCount++;

        // print progress
        cout << "Showing Boxes " << frameCount << "/" << totalFrames << endl;
        cout.flush();
    }
}

int main(int argc, char* argv[]){
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <blockSize> <maxBoxes> <whiteThreshold>" << endl;
        return 1;
    }

    blockSize = stoi(argv[1]);
    maxBoxInFrame = stoi(argv[2]);
    whiteThreshold = stod(argv[3]);

    blockSize = max(0, blockSize);
    maxBoxInFrame = max(0, maxBoxInFrame);
    whiteThreshold = max(0.0, min(1.0, whiteThreshold));

    string videoPath = "../../video/video.mp4";  // DO NOT change the video path
    string boxesPath = "../../doc/boxes.txt";  // DO NOT change the boxes path
    string framePath = "../frames";  // DO NOT change the frame path

    video2boxes(videoPath, boxesPath);
    showBoxesinFrame(videoPath, boxesPath, framePath);

    cout << "Done" << endl;
    return 0;
}
# C++ Version of Siren Environment.

## Pre-request
- OpenCV 3
- [swig](http://www.swig.org)
- [Boost Preprocessor Library](https://www.boost.org)

## Install
1. (If necessery) Copy `preprocessor` folder from boost into `src_cpp/boost/`
2. Add all .h file into my_module.i
3. Add all .cpp file before '-o' in the second line in `compile`
4. Run the command:

```
$ ./compile
```
 
#### The environment contains three classes, as described in the followings:
 
- **GTImage**: Ground truth image class. A instance of GTImage class here is a small matrix (with 14 rows, 19 columns as default), each element has 3 channels (Using OpenCV Mat type CV_8UC3). When creating a GTImage instance, it reads the \*real\* ground truth image from file system, down sampling and processing it to form a block/cell based chess board. This chess board is later used to find possible actions and give reward for the deep reinforcement learning agent.

    The \*real\* ground truth image is a combination of OSM road centerline data and building predictions from ICTNet (or other network architectures). Save these data into the chess board as [building, road, preserved_channel].
 
- **RGBImage**: RGB image class. This class is used to process the satellite imagery. When creating a instance of RGBImage class, it reads the satellite image from file system, down-sampling and do \f[some more\f] processing steps. At last, it generate a low resolution color image (170 x 160 pixels by default). This image is later feed into the deep neural network for training/testing (extract road centerlines).
 
- **BoardState**: board state class. This class plays the role of a game manager. It combines
    GTImage class and RGBImage class, update the current state, next state, finds possible
    actions, calculate rewards and check if the current extraction progress is done. It keeps
    tracking everything and returns all the necessery information back to gym environment.
    

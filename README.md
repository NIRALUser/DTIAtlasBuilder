<p align="right"><img width="15%" src="https://raw.githubusercontent.com/NIRALUser/DTIAtlasBuilder/master/Data/DTIAtlasBuilderIcon.jpeg"/></p>

# DTIAtlasBuilder

Current stable release: [**1.5**](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.5.0) (11/12/2018)

## What is it?

A tool to create an atlas from several DTI images

<img width="100%" src="https://raw.githubusercontent.com/NIRALUser/DTIAtlasBuilder/master/Data/dtiabshortpipe.png"/>

## Where is it?

Find the tool on [NITRC](http://www.nitrc.org/projects/dtiatlasbuilder "DTIAtlasBuilder on NITRC") and [GitHub](http://github.com/NIRALUser/DTIAtlasBuilder "DTIAtlasBuilder on GitHub")

Binary package available on the [NITRC download page](http://www.nitrc.org/frs/?group_id=636)

Usage tutorial available [from NITRC](http://www.nitrc.org/docman/view.php/636/1189/DTIAtlasBuilder_Tutorial.pdf)

## External Requirements
Following OS level library and tools needs to be installed before build

- python-devel
- qt4-qtbase-devel
- qt5-qtbase-devel
- freeglut-devel
- cmake 3.4 or above

These Softwares need to be installed before executing the tool :
- ImageMath
- ResampleDTIlogEuclidean
- CropDTI
- dtiprocess
- BRAINSFit
- GreedyAtlas
- dtiaverage
- DTI-Reg
- MriWatcher
- unu

If you do not have these softwares installed on your machine, you can use the COMPILE_PACKAGE CMake option to download and compile aumatically the tools you need. If you do so, please run "make install" after the compilation to copy the tools into the CMAKE_INSTALL_PREFIX.


## Dockerfile for developers

Use below command in source directory to build docker image (Currently having only CentOS7 image.

```
$ docker build . -t <image-name>
$ docker run --rm -it -v $PWD/../:/work -w /work <image-name> 
```


## Change Log:

#### [v1.5](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.5.0) (11/12/2018)

- Fix superbuild mechanism for SlicerExtension
- New mechanism to find executables based on Slicer architecture

#### [v1.3](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.3) (06/26/2013)

- File/Folder management:
  - Keep original subject names instead of using "CaseX"
  - Put final files in specific folders (FinalTensors and FinalDeformationFields)  
  - Automatically update files/folders names if running DTIAtlasBuilder over a previous computation done with an older version  
- Add step at the beginning to filter and set to zero negative values in tensors to avoid issues in registration  
- New Interface features:
  - Drag & Drop images and text files (CSV, parameters and software configuration files)
  - "Clean output folder" button
  - Keyboard shortcuts
  - Remove duplicated cases
  - Improve enable/disable buttons
- Other minor bug fixes

- Python version 2.5 minimum required

#### [v1.2](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.2) (05/01/2013)

- Fixing the output global displacement fields when using ANTs

- Files/Folders name changes:

```
2_NonLinear_Registration/CaseX_DeformationField.mhd                     -> 2_NonLinear_Registration/CaseX_HField.mhd
4_Final_Resampling/First_Resampling/CaseX_GlobalDeformationField.nrrd   -> 4_Final_Resampling/First_Resampling/CaseX_GlobalDisplacementField.nrrd
4_Final_Resampling/Second_Resampling/CaseX_GlobalDeformationField.nrrd  -> 4_Final_Resampling/Second_Resampling/CaseX_GlobalDisplacementField.nrrd
```

#### [v1.1](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.1) (02/22/2013)

- Add a SuperBuild system to automatically download and compile all needed external tools

- Add DTIAtlasBuilder as a [Slicer extension](http://www.slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/DTIAtlasBuilder)

- Replace AtlasWerks by GreedyAtlas

- Files/Folders name changes:

```
2_NonLinear_Registration_AW                  -> 2_NonLinear_Registration
2_NonLinear_Registration/AverageImage.mhd    -> 2_NonLinear_Registration/MeanImage.mhd
3_AW_Atlas                                   -> 3_Diffeomorphic_Atlas
3_Diffeomorphic_Atlas/CaseX_AWDTI.nrrd       -> 3_Diffeomorphic_Atlas/CaseX_DiffeomorphicDTI.nrrd
3_Diffeomorphic_Atlas/AWAtlasDTI.nrrd        -> 3_Diffeomorphic_Atlas/DiffeomorphicAtlasDTI.nrrd
```

#### [v1.0](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.0) (10/11/2012)

- Initial version


## Troubleshooting:

#### Final Atlas is bad

If you see in the QC windows that the Affine and Diffeomorphic Atlases and registered images look OK, and that the Final Atlas and images look bad for all or most cases, that means that something went wrong during the FINAL registration.  
If this final registration was done using BRAINS, try to recompute it using ANTS (in the Final Resampling tab > Registration Method), which gives a better registration.


#### Loading parameter file fail

If a parameter file fails to open in DTIAtlasBuilder (Parameter file is corrupted), you need to recreate it by loading only the corresponding dataset file and setting your options again.

#### QC does not show up

If the QC windows do not appear when pushing the QC buttons, it might mean that you need to install the GLUT library to get MriWatcher to work.  
You can find the GLUT library [here](http://www.opengl.org/resources/libraries/glut/glut_downloads.php)

#### Contact : jprieto[at]med.unc.edu

Slicer extension build status: [Open the dashboard](http://slicer.cdash.org/index.php?project=Slicer4) and search "DTIAtlasBuilder" in the build name using the filters.

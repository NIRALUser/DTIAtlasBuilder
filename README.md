#DTIAtlasBuilder

Current stable release: [**1.2**](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.2) (05/01/2013)

##What is it?

A tool to create an atlas from several DTI images

##Where is it?

Find the tool on [NITRC](http://www.nitrc.org/projects/dtiatlasbuilder "DTIAtlasBuilder on NITRC") and [GitHub](http://github.com/NIRALUser/DTIAtlasBuilder "DTIAtlasBuilder on GitHub")

Binary package available on the [NITRC download page](http://www.nitrc.org/frs/?group_id=636)

Usage tutorial available [from NITRC](http://www.nitrc.org/docman/view.php/636/1189/DTIAtlasBuilder_Tutorial.pdf)

##External Requirements

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

##Change Log:

####[v1.2](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.2) (05/01/2013)

- Fixing the output global displacement fields when using ANTs

- Files/Folders name changes:

```
2_NonLinear_Registration/CaseX_DeformationField.mhd                     -> 2_NonLinear_Registration/CaseX_HField.mhd
4_Final_Resampling/First_Resampling/CaseX_GlobalDeformationField.nrrd   -> 4_Final_Resampling/First_Resampling/CaseX_GlobalDisplacementField.nrrd
4_Final_Resampling/Second_Resampling/CaseX_GlobalDeformationField.nrrd  -> 4_Final_Resampling/Second_Resampling/CaseX_GlobalDisplacementField.nrrd
```

####[v1.1](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.1) (02/22/2013)

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

####[v1.0](http://github.com/NIRALUser/DTIAtlasBuilder/tree/v1.0) (10/11/2012)

- Initial version


##Troubleshooting:

####Load a parameter file

If a parameter file fails to open in DTIAtlasBuilder (Parameter file is corrupted), you need to recreate it by loading only the corresponding dataset file and setting your options again.

####Contact : akaiser[at]unc[dot]edu

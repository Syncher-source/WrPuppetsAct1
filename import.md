# WrPuppets Act 1

## Replacing the default model detailed instructions

It is **extremely important the model is in t-pose**, it can be done in Unreal too creating a pose asset but that would add more complexity, so just make sure your model is in t-pose before going on with the export.

Make also sure the forearms are slightly bended forward.

Before doing the export it is necessary to scale the model 100x:
Select the root often called Armature, press s, 100, and return, then appy the scale using ctrl+a and selecting "Scale"

![export0](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Export0.png)

Export the .fbx file

![export1](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Export1.png)

compared to the default Cats export settings you need to set under Geometry: Face Smooth

![export2](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Export2.png)

Import the .fbx into the reference WrPuppets Unreal Engine project, reset the import dialog to default values and than change:
Import Morph Targets: set tick
Normal Import Method: select Import Normals
Create Physics Asset: set untick

![import0](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Import0.png)

Make sure to clear skeleton if Unreal Engine automatically selects one

![import1](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Import1.png)

To fix the imported materials and texture is out of this tutorial.
Then you have to add a "Viewpoint" socket since it is necessary to define where the eyes of the importe models are:

![viewpoint](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Viewpoint.png)

This is used to scale the world taking this viewpoint as reference compared to the HMD position.
Now you will have to replace the binding from the default model to the imported assets:
Double click on WrPuppets_Anim, click "Class Settings" to open the Details pane

![switch0](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Switch0.png)

Check there for "Class Options", "Advanced", there you need to change the "Target Skeleton" to the imported one and compile, save and close the window.
If you don't close the window sometime it doesn't update promptly.

![switch1](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Switch1.png)

Then the mesh:

Now double click on WrPuppets_Pawn, select WrPuppets_SkeletalMesh on the Components 

![switch2](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Switch2.png)

and then on the Details panel select the imported skeletal mesh as "Skeletal Mesh Asset" and again compile, save and close the window.

![switch3](https://github.com/Syncher-source/WrPuppets/blob/main/README_images/Switch3.png)

You can now run the project.


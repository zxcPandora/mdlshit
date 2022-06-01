#include <conv.hpp>
#include <logger.hpp>
#include <structs.hpp>
#include <binarystream.hpp>
#include <structs.hpp>
#include <argtools.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <rendering/progress.hpp>
#include <vector>
using namespace std;

void CopyAddInt32(BinaryReader* reader, BinaryWriter* writer, int add, int count){
    for(int i = 0; i < count;i++){
        int tmp;
        reader->Read(&tmp);
        writer->Write(tmp + add);
    }
}
void CopyAddInt32NullCheck(BinaryReader* reader, BinaryWriter* writer, int add, int count){
    for(int i = 0; i < count;i++){
        int tmp;
        reader->Read(&tmp);
        if(tmp == 0)
          writer->Write(tmp);
        else
          writer->Write(tmp + add);
    }
}
void AddInt32(BinaryWriter* writer, int add, int count){
    for(int i = 0; i < count;i++){
        writer->Write(add);
    }
}
void CopyAddFloat32(BinaryReader* reader, BinaryWriter* writer, float add, int count){
    for(int i = 0; i < count;i++){
        float tmp;
        reader->Read(&tmp);
        writer->Write(tmp + add);
    }
}
void CopySetFloat32(BinaryReader* reader, BinaryWriter* writer, float set, int count){
    for(int i = 0; i < count;i++){
        float tmp;
        reader->Read(&tmp);
        writer->Write(set);
    }
}


string h(const char* file, const char* ext){
  string yo = string(file);
  auto dot = yo.find_last_of('.');
  yo.resize(dot);
  yo.append(".");
  yo.append(ext);
  return yo;
}

void filler(BinaryReader* reader, BinaryWriter* writer, int count){
  for(int i = 0;i < count;i++){
    byte tmp;
    reader->Read(&tmp);
    writer->Write(tmp);
  }
}

int Conversion::ReadHeader(FileInfo info) {

  UI::Progress.MainTask.Begin("Converting");

  BinaryReader Stream = BinaryReader(info.mdl.value().c_str());

  
  
  

  BinaryWriter OutStream = BinaryWriter(info.out.value().c_str());
  v53_Header*       Dest_Header =          new v53_Header();
  v53_Header_Part2* Dest_Header_Part2 =    new v53_Header_Part2();
  v49_Header*       Initial_Header =       new v49_Header();
  v53_Header_Part2* Initial_Header_Part2 = new v53_Header_Part2();
  Stream.seek(0);

  Stream.Read(&Initial_Header->id);
  Stream.Read(&Initial_Header->version);

  if(Initial_Header->version > 49){
    Logger::Critical("Model version (%d) is above maximum this program will allow (49), aborting...\n",Initial_Header->version);
  }

  Logger::Debug("Version: %d\n", Initial_Header->version);
  Stream.Read(&Initial_Header->checksum);
  Logger::Debug("checksum: %d\n", Initial_Header->checksum);
  Stream.read(Initial_Header->name, 64);
  Logger::Debug("name: %s\n", &Initial_Header->name);
  Stream.Read(&Initial_Header->fileSize);
  Logger::Debug("fileSize: %d\n", Initial_Header->fileSize);
  Stream.Read(&Initial_Header->eyeposition);
  Stream.Read(&Initial_Header->illumposition);
  Stream.Read(&Initial_Header->hull_min);
  Stream.Read(&Initial_Header->hull_max);
  Stream.Read(&Initial_Header->view_bbmin);
  Stream.Read(&Initial_Header->view_bbmax);
  Stream.Read(&Initial_Header->flags);
  Logger::Debug("flags: %d\n", Initial_Header->flags);
  Stream.Read(&Initial_Header->numbones);
  Logger::Debug("Bone count: %d\n", Initial_Header->numbones);
  Stream.Read(&Initial_Header->boneindex);
  Logger::Debug("Bones index: %d\n", Initial_Header->boneindex);
  Stream.Read(&Initial_Header->numbonecontrollers);
  Stream.Read(&Initial_Header->bonecontrollerindex);
  Stream.Read(&Initial_Header->numhitboxsets);
  Stream.Read(&Initial_Header->hitboxsetindex);
  Stream.Read(&Initial_Header->numlocalanim);
  Logger::Debug("Anim Count: %d\n",Initial_Header->numlocalanim);
  Stream.Read(&Initial_Header->localanimindex);
  Logger::Debug("Anim Index: %d\n",Initial_Header->localanimindex);

  Stream.Read(&Initial_Header->numlocalseq);
  Logger::Debug("Sequence Count: %d\n",Initial_Header->numlocalseq);
  Stream.Read(&Initial_Header->localseqindex);
  Logger::Debug("Sequence Index: %d\n",Initial_Header->localseqindex);

  Stream.Read(&Initial_Header->activitylistversion);
  Stream.Read(&Initial_Header->eventsindexed);

  Stream.Read(&Initial_Header->numtextures);
  Stream.Read(&Initial_Header->textureindex);
  Logger::Debug("Texture index: %d\n", Initial_Header->textureindex);

  Stream.Read(&Initial_Header->numcdtextures);
  Stream.Read(&Initial_Header->cdtextureindex);

  Stream.Read(&Initial_Header->numskinref);
  Stream.Read(&Initial_Header->numskinfamilies);
  Stream.Read(&Initial_Header->skinindex);

  Stream.Read(&Initial_Header->numbodyparts);
  Stream.Read(&Initial_Header->bodypartindex);
  Logger::Debug("Body part index: %d\n", Initial_Header ->bodypartindex);

  Stream.Read(&Initial_Header->numlocalattachments);
  Stream.Read(&Initial_Header->localattachmentindex);
  Logger::Debug("Local attachment index: %d\n", Initial_Header ->localattachmentindex);
  Stream.Read(&Initial_Header->numlocalnodes);
  Stream.Read(&Initial_Header->localnodeindex);
  Stream.Read(&Initial_Header->localnodenameindex);
  Stream.Read(&Initial_Header->numflexdesc);
  Stream.Read(&Initial_Header->flexdescindex);
  Stream.Read(&Initial_Header->numflexcontrollers);
  Stream.Read(&Initial_Header->flexcontrollerindex);
  Stream.Read(&Initial_Header->numflexrules);
  Stream.Read(&Initial_Header->flexruleindex);
  Stream.Read(&Initial_Header->numikchains);
  Stream.Read(&Initial_Header->ikchainindex);
  Stream.Read(&Initial_Header->nummouths);
  Stream.Read(&Initial_Header->mouthindex);
  Stream.Read(&Initial_Header->numlocalposeparameters);
  Stream.Read(&Initial_Header->localposeparamindex);
  Stream.Read(&Initial_Header->surfacepropindex);
  Stream.Read(&Initial_Header->keyvalueindex);
  Stream.Read(&Initial_Header->keyvaluesize);
  Stream.Read(&Initial_Header->numlocalikautoplaylocks);
  Stream.Read(&Initial_Header->localikautoplaylockindex);
  Stream.Read(&Initial_Header->mass);
  Stream.Read(&Initial_Header->contents);
  Stream.Read(&Initial_Header->numincludemodels);
  Logger::Debug("Included model count: %d\n", Initial_Header->numincludemodels);
  Stream.Read(&Initial_Header->includemodelindex);
  Logger::Debug("Included model index: %d\n", Initial_Header->includemodelindex);
  Stream.Read(&Initial_Header->virtualModel);
  Stream.Read(&Initial_Header->szanimblocknameindex);
  Stream.Read(&Initial_Header->numanimblocks);
  Stream.Read(&Initial_Header->animblockindex);
  Stream.Read(&Initial_Header->animblockModel);
  Stream.Read(&Initial_Header->bonetablebynameindex);
  Stream.Read(&Initial_Header->pVertexBase);
  Stream.Read(&Initial_Header->pIndexBase);
  Stream.Read(&Initial_Header->constdirectionallightdot);
  Stream.Read(&Initial_Header->rootLOD);
  Stream.Read(&Initial_Header->numAllowedRootLODs);
  Stream.Read(&Initial_Header->unused[0]);
  Stream.Read(&Initial_Header->unused4);
  Stream.Read(&Initial_Header->numflexcontrollerui);
  Stream.Read(&Initial_Header->flexcontrolleruiindex);
  Stream.Read(&Initial_Header->flVertAnimFixedPointScale);
  Stream.Read(&Initial_Header->surfacepropLookup);
  Stream.Read(&Initial_Header->studiohdr2index);
  Stream.Read(&Initial_Header->unused2[0]);

  Stream.seek(Initial_Header->studiohdr2index);

  Stream.Read(&Initial_Header_Part2->staticValues1[0]);
  Stream.Read(&Initial_Header_Part2->staticValues1[1]);
  Stream.Read(&Initial_Header_Part2->staticValues1[2]);
  Stream.Read(&Initial_Header_Part2->oneshyOfStringChunk);
  Stream.Read(&Initial_Header_Part2->vtxOffset1);
  Stream.Read(&Initial_Header_Part2->vvdOffset);
  Stream.Read(&Initial_Header_Part2->idcvOffset);
  Stream.Read(&Initial_Header_Part2->phyOffset);
  Stream.Read(&Initial_Header_Part2->staticValues2[0]);
  Stream.Read(&Initial_Header_Part2->staticValues2[1]);
  Stream.Read(&Initial_Header_Part2->staticValues2[2]);
  Stream.Read(&Initial_Header_Part2->staticValues2[3]);
  Stream.Read(&Initial_Header_Part2->unknownBlock);
  Stream.Read(&Initial_Header_Part2->unknown2[0]);
  Stream.Read(&Initial_Header_Part2->unknown2[1]);
  Stream.Read(&Initial_Header_Part2->unknown2[2]);
  Stream.Read(&Initial_Header_Part2->vtxOffset2);
  Logger::Notice("Finished reading at %d\n",Stream.Position());
  

  memcpy(Dest_Header->name, Initial_Header->name, sizeof(Initial_Header->name));


int TestDiff = (Initial_Header->numlocalanim * -8)
+
(Initial_Header->numlocalseq * 20)
+
(0);


int BytesAdded = 4;
int TextureDiff = (20 * Initial_Header->numtextures);
#pragma region bone conversion
int bone_filler = Initial_Header->boneindex - Stream.Position();
OutStream.seek(Stream.Position() + 4);
  filler(&Stream,&OutStream,bone_filler);
  Dest_Header->boneindex = OutStream.Position();
  UI::Progress.SubTask.Begin("Converting Bones");
  int bonecount = Initial_Header->numbones;
  for (int i = 0; i < bonecount; i++) {
      
    int boneOff = TestDiff + (28 * bonecount) - (28 * i) - TextureDiff;
    CopyAddInt32(&Stream, &OutStream, boneOff, 1); //sznameindex
    CopyAddInt32(&Stream, &OutStream, 0, 1); //parent
    CopyAddInt32(&Stream, &OutStream, 0, 6); //bonecontroller
    CopySetFloat32(&Stream, &OutStream, 1,3); //pos
    
    //quaternion
    CopyAddFloat32(&Stream, &OutStream, 0, 3);
    CopySetFloat32(&Stream, &OutStream, 1, 1); //quat
    //end quaternion
    CopyAddFloat32(&Stream, &OutStream, 0, 3); //rot
    CopySetFloat32(&Stream, &OutStream, 1,3); //posscale
    AddInt32(&OutStream, 0, 3); //addme
    CopySetFloat32(&Stream, &OutStream, 1,3); //rotscale
    AddInt32(&OutStream, 0, 3); //addme
    CopyAddInt32(&Stream, &OutStream, 0, 12); //posetobone
    //quaternion
    CopyAddFloat32(&Stream, &OutStream, 0, 3);
    CopySetFloat32(&Stream, &OutStream, 1, 1); //qalignment
    //end quaternion
    CopyAddInt32(&Stream, &OutStream, 0, 1); //flags
    CopyAddInt32(&Stream, &OutStream, 0, 1); //proctype
    CopyAddInt32(&Stream, &OutStream, 0, 1); //procindex
    CopyAddInt32(&Stream, &OutStream, 0, 1); //physicsbone
    CopyAddInt32(&Stream, &OutStream, 0, 1); //surfacepropidx
    CopyAddInt32(&Stream, &OutStream, 0, 1); //contents
    CopyAddInt32(&Stream, &OutStream, 0, 8); //unused
    AddInt32(&OutStream, 0, 1); //addme
    BytesAdded += 28;
    Logger::Notice("Converted bone %d of %d\n",i+1,bonecount);
    UI::Progress.SubTask.Update((i+1.0f)/(float)bonecount);
  }
  UI::Progress.SubTask.End();
Logger::Info("Finished bone conversion\n");
#pragma endregion
int SeqAdd = 20 * Initial_Header->numlocalseq;
int AnimYeet = -8 * Initial_Header->numlocalanim;



if(OutStream.Position() - BytesAdded != Stream.Position()){
  Logger::Error("Cursors were not at the same relative position after doing bones\n");
  Logger::Error("(%d vs %d)\n",(OutStream.Position() - BytesAdded),Stream.Position());
  return 1;
}
int BoneBytesAdded = BytesAdded + 0;

int att_filler_dest = Initial_Header->localattachmentindex - Stream.Position();
filler(&Stream, &OutStream, att_filler_dest);
Dest_Header->localattachmentindex = OutStream.Position();
UI::Progress.SubTask.Begin("Converting Attachments");
for(int i = 0; i < Initial_Header->numlocalattachments;i++){
  //92 bytes
  int idx;
  Stream.Read(&idx);
  int off = (idx)  + SeqAdd + AnimYeet;
  OutStream.Write(off);
  filler(&Stream, &OutStream, 88);
  Logger::Notice("Converted local attachment %d of %d\n",i+1,Initial_Header->numlocalattachments);
  UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numlocalattachments);
}
UI::Progress.SubTask.End();
Stream.seek(Stream.Position()+92);
OutStream.seek(OutStream.Position()+92);
int fuckshittwoelectricboogaloo;
Stream.Read(&fuckshittwoelectricboogaloo);
OutStream.Write(fuckshittwoelectricboogaloo);


//CopyAddInt32(&Stream, &OutStream, SeqAdd + AnimYeet, 1);

vector<pair<int,int>> ASSFARTS; //i am mature

//hitboxes
int hitbox_filler_dest = Initial_Header->hitboxsetindex - Stream.Position();
filler(&Stream, &OutStream, hitbox_filler_dest);
UI::Progress.SubTask.Begin("Converting Hitboxes");
for(int i = 0; i < Initial_Header->numhitboxsets;i++){
  int ass;
  Stream.Read(&ass);
  OutStream.Write(ass + OutStream.Position());//sznameindex
  //ASSFARTS.push_back(make_pair(ass,ass + OutStream.Position()-4));
  CopyAddInt32(&Stream, &OutStream, 0, 1); //numhitboxes
  CopyAddInt32(&Stream, &OutStream, SeqAdd + AnimYeet, 1); //hitboxindex
  
  Logger::Notice("Shifted hitbox set %d of %d\n",i+1,Initial_Header->numhitboxsets);
  UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numhitboxsets);
}
UI::Progress.SubTask.End();


//anims
int anim_filler_dest = Initial_Header->localanimindex - Stream.Position();
filler(&Stream, &OutStream, anim_filler_dest);
Dest_Header->localanimindex = OutStream.Position();
UI::Progress.SubTask.Begin("Converting Animations");
for(int i = 0; i < Initial_Header->numlocalanim;i++){
  int PISS = -((8*(Initial_Header->numlocalanim - i)));
  CopyAddInt32(&Stream, &OutStream, 0, 1); //baseptr
  CopyAddInt32NullCheck(&Stream, &OutStream, (PISS+SeqAdd)-TextureDiff, 1); //sznameindex
  CopyAddInt32(&Stream, &OutStream, 0, 1); //fps
  CopyAddInt32(&Stream, &OutStream, 0, 1); //flags
  CopyAddInt32(&Stream, &OutStream, 0, 1); //numframes
  CopyAddInt32(&Stream, &OutStream, 0, 1); //nummovements
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //movementindex
  int shitfuck[6];
  Stream.read((char*)&shitfuck, 4*6);
  CopyAddInt32(&Stream, &OutStream, 0, 1); //animblock
  CopyAddInt32NullCheck(&Stream, &OutStream, (PISS+SeqAdd+24)-TextureDiff, 1); //animindex
  CopyAddInt32(&Stream, &OutStream, 0, 1); //numikrules
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //ikruleindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //animblockikruleindex
  CopyAddInt32(&Stream, &OutStream, 0, 1); //numlocalhierarchy
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //localhierarchyindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //sectionindex
  CopyAddInt32(&Stream, &OutStream, 0, 1); //sectionframes
  CopyAddInt32(&Stream, &OutStream, 0, 1); //zeroframespan, zeroframecount
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //zeroframeindex
  

  CopyAddFloat32(&Stream, &OutStream, 0, 1);
  OutStream.write((char*)&shitfuck, 16);
  BytesAdded -= 8;
  Logger::Notice("Converted animation %d of %d\n",i+1,Initial_Header->numlocalanim);
  UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numlocalanim);
}
UI::Progress.SubTask.End();
Logger::Info("Finished animations\n");





//sequences
int seq_filler_dest = Initial_Header->localseqindex - Stream.Position();
filler(&Stream, &OutStream, seq_filler_dest);
Dest_Header->localseqindex = OutStream.Position();
UI::Progress.SubTask.Begin("Converting Sequences");
for(int i = 0; i < Initial_Header->numlocalseq;i++){
  int PISS = (TextureDiff + (20*(Initial_Header->numlocalseq - i)))-(8*Initial_Header->numlocalanim);
  CopyAddInt32(&Stream, &OutStream, 0, 1); //baseptr
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //szlabelindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //szactivitynameindex
  CopyAddInt32(&Stream, &OutStream, 0, 4);
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //eventindex
  CopyAddInt32(&Stream, &OutStream, 0, 3);   //bbmin
  CopyAddInt32(&Stream, &OutStream, 0, 3);   //bbmax
  CopyAddInt32(&Stream, &OutStream, 0, 1);   //numblends
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //animindexindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //movementindex
  CopyAddInt32(&Stream, &OutStream, 0, 2); //groupsize
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 2); //paramindex
  CopyAddInt32(&Stream, &OutStream, 0, 2); //paramstart
  CopyAddInt32(&Stream, &OutStream, 0, 2); //paramend
  CopyAddInt32(&Stream, &OutStream, 0, 1); //paramparent
  CopyAddInt32(&Stream, &OutStream, 0, 1); //fadeintime
  CopyAddInt32(&Stream, &OutStream, 0, 1); //fadeouttime
  CopyAddInt32(&Stream, &OutStream, 0, 1); //localentrynode
  CopyAddInt32(&Stream, &OutStream, 0, 1); //localexitnode
  CopyAddInt32(&Stream, &OutStream, 0, 1); //nodeflags
  CopyAddInt32(&Stream, &OutStream, 0, 1); //entryphase
  CopyAddInt32(&Stream, &OutStream, 0, 1); //exitphase
  CopyAddInt32(&Stream, &OutStream, 0, 1); //lastframe
  CopyAddInt32(&Stream, &OutStream, 0, 1); //nextseq
  CopyAddInt32(&Stream, &OutStream, 0, 1); //pose
  CopyAddInt32(&Stream, &OutStream, 0, 1); //numikrules
  CopyAddInt32(&Stream, &OutStream, 0, 1); //numautolayers  
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //autolayerindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //weightlistindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //posekeyindex
  CopyAddInt32(&Stream, &OutStream, 0, 3);
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //iklockindex
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //keyvalueindex
  CopyAddInt32(&Stream, &OutStream, 0, 1); //keyvaluesize
  CopyAddInt32NullCheck(&Stream, &OutStream, PISS, 1); //cycleposeindex
  int unused[5];
  Stream.read((char*)&unused, 20);
  OutStream.Write(0); //activityModifierOffset
  OutStream.Write(0); //activityModifierCount
  OutStream.write((char*)&unused, 20);
  OutStream.Write(0);
  OutStream.Write(0);
  OutStream.Write(0);
  BytesAdded += 20;


  Logger::Notice("Converted sequence %d of %d\n",i+1,Initial_Header->numlocalseq);
  UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numlocalseq);
}
UI::Progress.SubTask.End();
Logger::Info("Finished sequences\n");

int bpart_filler_dest = Initial_Header->bodypartindex - Stream.Position();
UI::Progress.SubTask.Begin("Converting Body Parts");
filler(&Stream, &OutStream, bpart_filler_dest);

for(int i = 0; i < Initial_Header->numbodyparts;i++){
  //16 bytes
  int idx;
  Stream.Read(&idx);
  int off = (idx) - TextureDiff;
  OutStream.Write(off);
  filler(&Stream, &OutStream, 12);
  Logger::Notice("Converted body part %d of %d\n",i+1,Initial_Header->numbodyparts);
  UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numbodyparts);
}
UI::Progress.SubTask.End();
Logger::Info("Finished body parts\n");

int inc_filler_dest = Initial_Header->includemodelindex - Stream.Position();
UI::Progress.SubTask.Begin("Converting Included Models");
filler(&Stream, &OutStream, inc_filler_dest);


filler(&Stream, &OutStream, 4);
for(int i = 0; i < Initial_Header->numincludemodels;i++){
  //8 bytes
  int idx;
  Stream.Read(&idx);
  int off = (idx) - (TextureDiff);
  OutStream.Write(off);
  filler(&Stream, &OutStream, 4);
  Logger::Notice("Converted included model %d of %d\n",i+1,Initial_Header->numincludemodels);
  UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numincludemodels);
}
Stream.seek(Stream.Position() - 4);
OutStream.seek(OutStream.Position() - 4);
UI::Progress.SubTask.End();
Logger::Info("Finished included models\n");

#pragma region texture conversion

int t_filler_dest = Initial_Header->textureindex - Stream.Position();
UI::Progress.SubTask.Begin("Converting Textures");
filler(&Stream,&OutStream,t_filler_dest);
int beginposR = Stream.Position();
int beginposW = OutStream.Position();
Dest_Header->textureindex = OutStream.Position();
  for(int i = 0; i < Initial_Header->numtextures;i++){
    int tBeginW = OutStream.Position();
    CopyAddInt32(&Stream, &OutStream, -((20)*(Initial_Header->numtextures - i)), 1);
    for(int j = 0;j < 40;j++){
      byte tmp;
      Stream.Read(&tmp);
      OutStream.Write(tmp);
    }
    Stream.seek(Stream.Position() + 20);
    BytesAdded -= 20;
    Logger::Notice("Converted texture %d of %d\n",i+1,Initial_Header->numtextures);
    UI::Progress.SubTask.Update((i+1.0f)/(float)Initial_Header->numtextures);
  }
  UI::Progress.SubTask.End();
Logger::Info("Finished textures\n");
#pragma endregion

Logger::Notice("Write cursor is %d bytes short of original texture index\n",OutStream.Position() - Initial_Header->textureindex);
Logger::Notice("%d bytes were added so far\n",BytesAdded);

if(OutStream.Position() - beginposW != (Initial_Header->numtextures * 44)){
  Logger::Critical("Texture conversion did not adjust by the proper amount of bytes\n");
  Logger::Critical("(%d vs %d)\n",(Initial_Header->numtextures * 44),OutStream.Position() - beginposW);
  return 1;
}

if(Stream.Position() - beginposR != (Initial_Header->numtextures * 64)){
  Logger::Critical("Read stream did not pick up the correct amount of bytes\n");
  Logger::Critical("(%d vs %d)\n",(Initial_Header->numtextures * 64),Stream.Position() - beginposR);
  return 1;
}


if(OutStream.Position() - BytesAdded != Stream.Position()){
  Logger::Critical("Cursors were not at the same relative position after doing textures\n");
  Logger::Critical("(%d vs %d)\n",(OutStream.Position() - BytesAdded),Stream.Position());
  return 1;
}


if(OutStream.Position() - (Initial_Header->numtextures * 44) != beginposW){
  Logger::Critical("Write cursor was not at the end of the textures\n");
  Logger::Critical("(%d vs %d)\n",(OutStream.Position()),Initial_Header->textureindex + (Initial_Header->numtextures * 44));
  return 1;
}






#pragma region rest of the file
int pos = Stream.Position();
UI::Progress.SubTask.Begin("Copying Misc Data");
for(int i = 0;i < Stream.size - pos;i++){
  byte tmp;
  Stream.Read(&tmp);
  OutStream.Write(tmp);
  UI::Progress.SubTask.Update((i+1.0f)/(float)(Stream.size-pos));
}
UI::Progress.SubTask.End();
if(OutStream.Position() < Stream.size + BytesAdded){
  Logger::Critical("Not enough of the file was written\n");
  Logger::Critical("(%d vs %d)\n",OutStream.Position(),Stream.size + BytesAdded);
  return 1;
}

Logger::Notice("Wrote Rest of File\n");
if(OutStream.Position() < Stream.Position() + BytesAdded){
  Logger::Critical("Relative cursors are desynced\n");
  Logger::Critical("(%d vs %d)\n",OutStream.Position(),Stream.Position() + BytesAdded);
  return 1;
}

  if(info.phy.has_value()){
    BinaryReader PhyStream = BinaryReader(info.phy.value().c_str());
    if(!PhyStream.Stream.good()){
      Logger::Error("Model's phy file does not exist, please ensure %s exists, and is located in the same directory as the file\n",info.phy.value().c_str());
      return 1;
    }
    Logger::Info("writing phy at [%d]...\n",OutStream.Position());
    Dest_Header_Part2->phyOffset = OutStream.Position();
    UI::Progress.SubTask.Begin("Copying Physics Data");
    for(int i = 0; i < PhyStream.size;i++){
      byte tmp;
      PhyStream.Read(&tmp);
      OutStream.Write(tmp);
      UI::Progress.SubTask.Update((i+1.0f)/(float)PhyStream.size);
    }
    UI::Progress.SubTask.End();
    Logger::Info("done\n");
  }


  

  if(info.vtx.has_value()){
    BinaryReader VtxStream = BinaryReader(info.vtx.value().c_str());
    if(!VtxStream.Stream.good()){
      Logger::Error("Model's vtx file does not exist, please ensure %s exists, and is located in the same directory as the file\n",info.vtx.value().c_str());
      return 1;
    }
    Logger::Info("writing vtx at [%d]...\n",OutStream.Position());
    Dest_Header_Part2->vtxOffset1 = OutStream.Position();
    Dest_Header_Part2->vtxOffset2 = OutStream.Position();
    Dest_Header_Part2->unknownBlock = OutStream.Position();
    UI::Progress.SubTask.Begin("Copying Mesh Data");
    for(int i = 0; i < VtxStream.size;i++){
      byte tmp;
      VtxStream.Read(&tmp);
      OutStream.Write(tmp);
      UI::Progress.SubTask.Update((i+1.0f)/(float)VtxStream.size);
    }
    UI::Progress.SubTask.End();
    Logger::Info("done\n");
  }

  if(info.vvd.has_value()){
    BinaryReader VvdStream = BinaryReader(info.vvd.value().c_str());
    if(!VvdStream.Stream.good()){
      Logger::Error("Model's vvd file does not exist, please ensure %s exists, and is located in the same directory as the file\n",info.vvd.value().c_str());
      return 1;
    }
    Logger::Info("writing vvd at [%d]...\n",OutStream.Position());
    Dest_Header_Part2->vvdOffset = OutStream.Position();
    UI::Progress.SubTask.Begin("Copying Vertex Data");
    for(int i = 0; i < VvdStream.size;i++){
      byte tmp;
      VvdStream.Read(&tmp);
      OutStream.Write(tmp);
      UI::Progress.SubTask.Update((i+1.0f)/(float)VvdStream.size);
    }
    UI::Progress.SubTask.End();
    Logger::Info("done\n");
  }

Logger::Notice("Converting header...\n");

#pragma endregion

int NameCopy = Initial_Header->szanimblocknameindex + BytesAdded;

#pragma region converting to 53

  Dest_Header->nameCopyOffset = NameCopy;
  Dest_Header->id = Initial_Header->id;
  Dest_Header->version = 53;
  Dest_Header->checksum = Initial_Header->checksum;
  Dest_Header->fileSize = Initial_Header->fileSize;
  Dest_Header->eyeposition = Initial_Header->eyeposition;
  Dest_Header->illumposition = Initial_Header->illumposition;
  Dest_Header->hull_min = Initial_Header->hull_min;
  Dest_Header->hull_max = Initial_Header->hull_max;
  Dest_Header->view_bbmin = Initial_Header->view_bbmin;
  Dest_Header->view_bbmax = Initial_Header->view_bbmax;
  Dest_Header->flags = (info.flags_override.has_value()) ? info.flags_override.value() : 0;
  Dest_Header->numbones = Initial_Header->numbones;
  //Dest_Header->boneindex = Initial_Header->boneindex + 4;
  Dest_Header->numbonecontrollers = Initial_Header->numbonecontrollers;
  Dest_Header->bonecontrollerindex = Initial_Header->bonecontrollerindex + BoneBytesAdded;
  Dest_Header->numhitboxsets = Initial_Header->numhitboxsets;
  Dest_Header->hitboxsetindex = Initial_Header->hitboxsetindex + BoneBytesAdded;
  Stream.seek(Initial_Header->hitboxsetindex);
  int hbsidx;
  Stream.Read(&hbsidx);
  OutStream.seek(Dest_Header->hitboxsetindex); OutStream.Write(hbsidx - TextureDiff);
  Stream.seek(Initial_Header->hitboxsetindex + 44);
  int hbsidx2;
  Stream.Read(&hbsidx2);
  OutStream.seek(Dest_Header->hitboxsetindex + 44); OutStream.Write(hbsidx2 - TextureDiff);
  Dest_Header->numlocalanim = (info.animation_override.has_value()) ? info.animation_override.value() : Initial_Header->numlocalanim;
  //Dest_Header->localanimindex = ((Initial_Header->numanimblocks <= 0) ? Initial_Header->bodypartindex + BoneBytesAdded : Initial_Header->localanimindex + BoneBytesAdded);//- (20 * Initial_Header->numtextures);
  Dest_Header->numlocalseq = (info.sequence_override.has_value()) ? info.sequence_override.value() : Initial_Header->numlocalseq;
  //Dest_Header->localseqindex = ((Initial_Header->numanimblocks <= 0) ? Initial_Header->bodypartindex + BoneBytesAdded : Initial_Header->localseqindex + BoneBytesAdded);// - (20 * Initial_Header->numtextures);
  Dest_Header->activitylistversion = Initial_Header->activitylistversion;
  Dest_Header->eventsindexed = Initial_Header->eventsindexed;
  Dest_Header->numtextures = Initial_Header->numtextures;
  //Dest_Header->textureindex = Initial_Header->textureindex + BoneBytesAdded;
  Dest_Header->numcdtextures = 1;
  Dest_Header->cdtextureindex = Initial_Header->cdtextureindex + BytesAdded;
  OutStream.seek(Dest_Header->cdtextureindex); OutStream.Write(NameCopy);
  Dest_Header->numskinref = Initial_Header->numskinref;
  Dest_Header->numskinfamilies = 1;
  Dest_Header->skinindex = Initial_Header->skinindex + BytesAdded;
  Dest_Header->numbodyparts = Initial_Header->numbodyparts;
  Dest_Header->bodypartindex = Initial_Header->bodypartindex + BytesAdded + TextureDiff;
  Dest_Header->numlocalattachments = Initial_Header->numlocalattachments;
  Dest_Header->localattachmentindex = Initial_Header->localattachmentindex + BytesAdded + TextureDiff;
  Dest_Header->numlocalnodes = Initial_Header->numlocalnodes;
  Dest_Header->localnodeindex = Initial_Header->localnodeindex + BytesAdded;
  Dest_Header->localnodenameindex = Initial_Header->localnodenameindex + BytesAdded;
  Dest_Header->numflexdesc = Initial_Header->numflexdesc;
  Dest_Header->flexdescindex = Initial_Header->flexdescindex + BytesAdded + TextureDiff;
  Dest_Header->numflexcontrollers = Initial_Header->numflexcontrollers;
  Dest_Header->flexcontrollerindex = Initial_Header->flexcontrollerindex + BytesAdded + TextureDiff;
  Dest_Header->numflexrules = Initial_Header->numflexrules;
  Dest_Header->flexruleindex = Initial_Header->flexruleindex + BytesAdded + TextureDiff;
  Dest_Header->numikchains = 0;
  Dest_Header->ikchainindex = Initial_Header->ikchainindex + BytesAdded + TextureDiff;
  Dest_Header->nummouths = Initial_Header->nummouths;
  Dest_Header->mouthindex = Initial_Header->mouthindex + BytesAdded + TextureDiff;
  Dest_Header->numlocalposeparameters = Initial_Header->numlocalposeparameters;
  Dest_Header->localposeparamindex = Initial_Header->localposeparamindex + BytesAdded;
  Dest_Header->surfacepropindex = Initial_Header->surfacepropindex + BytesAdded;
  Dest_Header->keyvalueindex = Initial_Header->keyvalueindex + BytesAdded;
  Dest_Header->keyvaluesize = Initial_Header->keyvaluesize;
  Dest_Header->numlocalikautoplaylocks = Initial_Header->numlocalikautoplaylocks;
  Dest_Header->localikautoplaylockindex = Initial_Header->localikautoplaylockindex + BytesAdded + TextureDiff;
  Dest_Header->mass = 0;
  Dest_Header->contents = Initial_Header->contents;
  Dest_Header->numincludemodels = Initial_Header->numincludemodels;
  Dest_Header->includemodelindex = Initial_Header->includemodelindex + BytesAdded + TextureDiff;
  Dest_Header->virtualModel = Initial_Header->virtualModel;
  Dest_Header->szanimblocknameindex = Initial_Header->bonetablebynameindex + BytesAdded + TextureDiff; // aa
  Dest_Header->numanimblocks = Initial_Header->numanimblocks;
  Dest_Header->animblockindex = -1082130432;
  Dest_Header->animblockModel = Initial_Header->animblockModel;
  Dest_Header->bonetablebynameindex = Initial_Header->bonetablebynameindex + BytesAdded + TextureDiff; // aa
  Dest_Header->pVertexBase = Initial_Header->pVertexBase;
  Dest_Header->pIndexBase = Initial_Header->pIndexBase;
  Dest_Header->constdirectionallightdot = (byte)0xCC;
  Dest_Header->rootLOD = (byte)0x02;
  Dest_Header->numAllowedRootLODs = Initial_Header->numAllowedRootLODs;
  memcpy(Dest_Header->unused, Initial_Header->unused,
         sizeof(Dest_Header->unused));
  Dest_Header->unused4 =
      (Initial_Header->version < 47) ? 0 : Initial_Header->unused4;
  Dest_Header->numflexcontrollerui = 0;   // notable
  Dest_Header->flexcontrolleruiindex = 0; // notable
  Dest_Header->flVertAnimFixedPointScale =
      Initial_Header->flVertAnimFixedPointScale;
  Dest_Header->surfacepropLookup = Initial_Header->surfacepropLookup;
  Dest_Header->studiohdr2index = Initial_Header->studiohdr2index + 4;
  memcpy(Dest_Header->unused2, Initial_Header->unused2,
         sizeof(Dest_Header->unused2));



  int zeroes[3] = {0, 0, 0};

  memcpy(Dest_Header_Part2->staticValues1, zeroes, sizeof(int) * 3);
  Dest_Header_Part2->oneshyOfStringChunk = Initial_Header->szanimblocknameindex + BytesAdded;
  //Dest_Header_Part2->vtxOffset1 = Initial_Header_Part2->vtxOffset1;
  //Dest_Header_Part2->vvdOffset = Initial_Header_Part2->vvdOffset;
  Dest_Header_Part2->idcvOffset = Initial_Header_Part2->idcvOffset;
  //Dest_Header_Part2->phyOffset = Initial_Header_Part2->phyOffset;
  memcpy(Dest_Header_Part2->staticValues2, Initial_Header_Part2->staticValues2,
         sizeof(Initial_Header_Part2->staticValues2));
  //Dest_Header_Part2->unknownBlock = Initial_Header_Part2->vtxOffset1;
  memcpy(Dest_Header_Part2->unknown2, zeroes, sizeof(int) * 3);
  //Dest_Header_Part2->vtxOffset2 = Initial_Header_Part2->vtxOffset2;
#pragma endregion

  Logger::Notice("done\n");
  Logger::Notice("Writing header...\n");

  OutStream.seek(0);
  OutStream.Write(Dest_Header->id);
  OutStream.Write(Dest_Header->version);
  OutStream.Write(Dest_Header->checksum);
  OutStream.Write(Dest_Header->nameCopyOffset);
  OutStream.write(Dest_Header->name, 64);
  OutStream.Write(Dest_Header->fileSize);
  OutStream.Write(Dest_Header->eyeposition);
  OutStream.Write(Dest_Header->illumposition);
  OutStream.Write(Dest_Header->hull_min);
  OutStream.Write(Dest_Header->hull_max);
  OutStream.Write(Dest_Header->view_bbmin);
  OutStream.Write(Dest_Header->view_bbmax);
  OutStream.Write(Dest_Header->flags);
  OutStream.Write(Dest_Header->numbones);
  OutStream.Write(Dest_Header->boneindex);
  OutStream.Write(Dest_Header->numbonecontrollers);
  OutStream.Write(Dest_Header->bonecontrollerindex);
  OutStream.Write(Dest_Header->numhitboxsets);
  OutStream.Write(Dest_Header->hitboxsetindex);
  OutStream.Write(Dest_Header->numlocalanim);
  OutStream.Write(Dest_Header->localanimindex);
  OutStream.Write(Dest_Header->numlocalseq);
  OutStream.Write(Dest_Header->localseqindex);
  OutStream.Write(Dest_Header->activitylistversion);
  OutStream.Write(Dest_Header->eventsindexed);
  OutStream.Write(Dest_Header->numtextures);
  OutStream.Write(Dest_Header->textureindex);
  OutStream.Write(Dest_Header->numcdtextures);
  OutStream.Write(Dest_Header->cdtextureindex);
  OutStream.Write(Dest_Header->numskinref);
  OutStream.Write(Dest_Header->numskinfamilies);
  OutStream.Write(Dest_Header->skinindex);
  OutStream.Write(Dest_Header->numbodyparts);
  OutStream.Write(Dest_Header->bodypartindex);
  OutStream.Write(Dest_Header->numlocalattachments);
  OutStream.Write(Dest_Header->localattachmentindex);
  OutStream.Write(Dest_Header->numlocalnodes);
  OutStream.Write(Dest_Header->localnodeindex);
  OutStream.Write(Dest_Header->localnodenameindex);
  OutStream.Write(Dest_Header->numflexdesc);
  OutStream.Write(Dest_Header->flexdescindex);
  OutStream.Write(Dest_Header->numflexcontrollers);
  OutStream.Write(Dest_Header->flexcontrollerindex);
  OutStream.Write(Dest_Header->numflexrules);
  OutStream.Write(Dest_Header->flexruleindex);
  OutStream.Write(Dest_Header->numikchains);
  OutStream.Write(Dest_Header->ikchainindex);
  OutStream.Write(Dest_Header->nummouths);
  OutStream.Write(Dest_Header->mouthindex);
  OutStream.Write(Dest_Header->numlocalposeparameters);
  OutStream.Write(Dest_Header->localposeparamindex);
  OutStream.Write(Dest_Header->surfacepropindex);
  OutStream.Write(Dest_Header->keyvalueindex);
  OutStream.Write(Dest_Header->keyvaluesize);
  OutStream.Write(Dest_Header->numlocalikautoplaylocks);
  OutStream.Write(Dest_Header->localikautoplaylockindex);
  OutStream.Write(Dest_Header->mass);
  OutStream.Write(Dest_Header->contents);
  OutStream.Write(Dest_Header->numincludemodels);
  OutStream.Write(Dest_Header->includemodelindex);
  OutStream.Write(Dest_Header->virtualModel);
  OutStream.Write(Dest_Header->szanimblocknameindex);

  OutStream.Write(Dest_Header->numanimblocks);
  OutStream.Write(Dest_Header->animblockindex);
  OutStream.Write(Dest_Header->animblockModel);
  OutStream.Write(Dest_Header->bonetablebynameindex);
  OutStream.Write(Dest_Header->pVertexBase);
  OutStream.Write(Dest_Header->pIndexBase);
  OutStream.Write(Dest_Header->constdirectionallightdot);
  OutStream.Write(Dest_Header->rootLOD);
  OutStream.Write(Dest_Header->numAllowedRootLODs);
  OutStream.Write(Dest_Header->unused[0]);
  OutStream.Write(Dest_Header->unused4);
  OutStream.Write(Dest_Header->numflexcontrollerui);
  OutStream.Write(Dest_Header->flexcontrolleruiindex);
  OutStream.Write(Dest_Header->flVertAnimFixedPointScale);
  OutStream.Write(Dest_Header->studiohdr2index);
  OutStream.Write(Dest_Header->unused2[0]);
  OutStream.seek(Dest_Header->studiohdr2index);
  OutStream.Write(Dest_Header_Part2->staticValues1[0]);
  OutStream.Write(Dest_Header_Part2->staticValues1[1]);
  OutStream.Write(Dest_Header_Part2->staticValues1[2]);
  OutStream.Write(Dest_Header_Part2->oneshyOfStringChunk);
  OutStream.Write(Dest_Header_Part2->vtxOffset1);
  OutStream.Write(Dest_Header_Part2->vvdOffset);
  OutStream.Write(Dest_Header_Part2->idcvOffset);
  OutStream.Write(Dest_Header_Part2->phyOffset);
  OutStream.Write(Dest_Header_Part2->staticValues2[0]);
  OutStream.Write(Dest_Header_Part2->staticValues2[1]);
  OutStream.Write(Dest_Header_Part2->staticValues2[2]);
  OutStream.Write(Dest_Header_Part2->staticValues2[3]);
  OutStream.Write(Dest_Header_Part2->unknownBlock);
  OutStream.Write(Dest_Header_Part2->unknown2[0]);
  OutStream.Write(Dest_Header_Part2->unknown2[1]);
  OutStream.Write(Dest_Header_Part2->unknown2[2]);
  OutStream.Write(Dest_Header_Part2->vtxOffset2);








  Logger::Notice("done\n");


  Logger::Notice("writing shitty fucking offsets that were too stupid to actually be normal and fuck this bro the array is nammed \"ASSFARTS\" lmao\n");
  if(ASSFARTS.size() >0) UI::Progress.SubTask.Begin("Copying stupid offsets");
  for(int i = 0; i < ASSFARTS.size();i++){
    OutStream.seek(0);
    Stream.seek(0);
    Stream.seek(ASSFARTS[i].first);
    int val;
    Stream.Read(&val);
    OutStream.seek(ASSFARTS[i].second);
    OutStream.Write(val);
    Logger::Notice("FUCK YOU, %d OF %d\n",i+1,ASSFARTS.size());
    UI::Progress.SubTask.Update((i+1.0f)/(float(ASSFARTS.size())));
  }
  if(ASSFARTS.size()>0) UI::Progress.SubTask.End();

  Logger::Notice("done\n");


  Logger::Info("Finished!\n");
  UI::Progress.MainTask.End();
  return 0;
}
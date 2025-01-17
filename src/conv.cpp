#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
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
#include <half.hpp>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <mdls.hpp>
#include <JsonTest.h>
#include <MLUtil.h>

//using namespace std;
using namespace rapidjson;

#pragma region helper functions

/// \param reader The input file
/// \param writer The output file
/// \param add The number to add to the int that was read
/// \param count Amount of times to repeat this
void AddInt32(BinaryWriter* writer, int add, int count){
    for(int i = 0; i < count;i++){
        writer->Write(add);
    }
}

void filler(BinaryReader* reader, BinaryWriter* writer, int count){
  for(int i = 0;i < count;i++){
    std::byte tmp;
    reader->Read(&tmp);
    writer->Write(tmp);
  }
}

void fillerWrite(BinaryWriter* writer, int count) {
    for (int i = 0; i < count; i++) {
        std::byte tmp = (std::byte)0;
        writer->Write(tmp);
    }
}
#pragma endregion

const int compact_range = 20000;

short compactFloat(double input) {
    return round(input * compact_range / 1000);
}
double expandToFloat(short input) {
    return ((double)input) * 1000 / compact_range;
}

int ConvertV52(FileInfo info)
{
    Logger::Debug("Conversion started\n");
    UI::Progress.MainTask.Begin("Converting");

    BinaryReader Stream = BinaryReader(info.mdl.value().c_str());
    BinaryWriter OutStream = BinaryWriter(info.out.value().c_str());
    JsonTest jsonController{}; //Made it class because why not. -Liberty
    //Logger::Info("Test\n");
    MDL::PHY vPhy{};

    if (info.phy.has_value()) {
        //Logger::Info("Test\n");
        BinaryReader PhyStream = BinaryReader(info.phy.value().c_str());
        if (!PhyStream.Stream.good()) {
            Logger::Error("Model's phy file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.phy.value().c_str());
            return 1;
        }
        //Logger::Info("Test\n");
        PhyStream.seek(0);
        vPhy = vPhy._PHY(&PhyStream, false);
        Logger::Info("done\n");
    }

    //Logger::Info("Test\n");
    //Logger::Info("Test\n");
    if (OutStream.Position() > 0) OutStream.seek(0);
    if (Stream.Position() > 0) Stream.seek(0);

    MDL::v52Mdl mdl = mdl._v52Mdl(&Stream, false);
    mdl.SetMdlInts();
    studiohdr_t_v53 v53Hdr = mdl.ConvertHeader(info);
    OutStream.Write(v53Hdr);
    mdl.UpdateMdl();

    std::vector<int> bytesAddedPerRuiMesh;
    std::vector<mstudioruimesh_t> ruiMeshes;

    std::vector<int> hdrBytesAnimDescAdd = mdl.v53GetAnimHdrBytesAdded(true);
    std::vector<int> secHdrBytesAnimDescAdd = mdl.v53GetSecHdrBytesAdded(true);
    int animByteAddedTotal = mdl.v53GetTotalAnimHdrBytesAdded();
    int animSecByteAddedTotal = mdl.v53GetTotalSecHdrBytesAdded();

    std::vector<int> secHdrBytesSecAdd = mdl.v53GetSecBytesAdded(true);
    std::vector<int> ikChainBones = mdl.GetIkChainBones();
    //std::vector<int> deltaAnims = GetDeltaAnims(&Stream, Initial_Header, false);

    int animBytesAdded = animByteAddedTotal + animSecByteAddedTotal;
    int bytesAddedToRuiMesh = 0;
    int bytesAddedToIkRules = -12 * mdl.iklinks.size();
    int bytesAddedToHeader = 52;
    int bytesAddedToBones = mdl.mdlhdr.numbones * 28;
    int bytesAddedToAnims = -8 * mdl.mdlhdr.numlocalanim;
    int bytesAddedToAnimData = mdl.mdlhdr.numlocalanim > 0 ? animByteAddedTotal + animSecByteAddedTotal + bytesAddedToIkRules : 0;
    int bytesAddedToSeqs = 20 * mdl.mdlhdr.numlocalseq;
    int bytesAddedToTextures = -20 * mdl.mdlhdr.numtextures;
    int bytesAddedToIkChains = 16 * mdl.mdlhdr.numikchains;
    int bytesAddedToActMods = 4 * mdl.activitymodifiers.size();
    int bytesAddedToLinearBone = -12 * mdl.mdlhdr.numbones;
    int textureFiller = 0;
    int strFiller = 60;
    int allBytesAdded = bytesAddedToHeader + bytesAddedToBones + bytesAddedToAnims + bytesAddedToSeqs + bytesAddedToTextures + bytesAddedToIkChains + bytesAddedToAnimData + bytesAddedToActMods + textureFiller + bytesAddedToRuiMesh + bytesAddedToLinearBone;

    jsonController.ReadExtraStructs(info);
    std::vector<mstudioanim_t_v53> anims = mdl.ConvertAnims();
    std::vector<mstudioanim_t_v53> sections = mdl.ConvertSections();
    std::vector<mstudiobone_t_v53> bones = mdl.BoneConversion();
    std::vector<mstudiobbox_t_v53> hitboxes = mdl.HitboxConversion();
    std::vector<mstudioanimdesc_t_v53> animdescs = mdl.AnimDescConversion();
    std::vector<sectionindexes_t_v53> sectionIndexes = mdl.ConvertSectionIndexes();
    std::vector<mstudioikrule_t_v53> ikRules = mdl.IkRuleConversion();
    std::vector<mstudioseqdesc_t_v53> seqdescs = mdl.SeqDescConversion();
    std::vector<mstudioikchain_t_v53> ikChains = mdl.IkChainConversion();
    std::vector<mstudiotexture_t_v53> textures = mdl.ConvertTextures();
    mstudiolinearbonedata_t_v53       linearBoneData = mdl.ConvertLinearBoneData();

    if (v53Hdr.boneindex > v53Hdr.mayaindex && v53Hdr.mayaindex != 0)
    {
        OutStream.seek(v53Hdr.mayaindex);
        OutStream.Write(mdl.mayabakery);
    }



    if (mdl.mdlhdr.numbones > 0)
    {
        int boneId = 0;
        OutStream.seek(v53Hdr.boneindex);
        UI::Progress.SubTask.Begin("Converting Bones");
        for (int i = 0; i < mdl.mdlhdr.numbones; i++)
        {
            int boneNum = i + 1;
            mstudiobone_t_v53 bone = bones[i];

            for (int j = 0; j < ikChainBones.size(); j++)
            {
                bool hasIkFlag = bone.flags & 0x20;
                if (i == ikChainBones[j] && !hasIkFlag) bone.flags += 0x20;
            }

            if (info.phy.has_value())
            {
                for (int k = 0; k < vPhy.phyHdr.numsolids; k++)
                {
                    if (boneNum == vPhy.phySecs[k].trihdr.parent)
                    {
                        //Logger::Info("boneidStart: %d\n", boneId);
                        bone.unkid = boneId;
                        bone.unktoggle = 1;
                        boneId++;

                        //Logger::Info("bone: %d\n", i);
                        //Logger::Info("phyParent: %d\n", vPhy.phySecs[k].trihdr.parent);
                        //Logger::Info("boneid: %d\n", bone.unkid);
                    }
                }
            }

            OutStream.Write(bone);

            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numbones);
        }
        UI::Progress.SubTask.End();
    }

    if (mdl.jigglebones.size() > 0)
    {
        for (int i = 0; i < mdl.jigglebones.size(); i++)
        {
            mstudiojigglebone_t_v49 jiggleBone = mdl.jigglebones[i];

            OutStream.Write(jiggleBone);
        }
    }

    if (mdl.mdlhdr.numlocalattachments > 0)
    {
        OutStream.seek(v53Hdr.localattachmentindex);
        UI::Progress.SubTask.Begin("Converting Attachments");
        for (int i = 0; i < v53Hdr.numlocalattachments; i++)
        {
            OutStream.Write(mdl.attachments[i]);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalattachments);
        }
        UI::Progress.SubTask.End();
    }

    //hitboxsets
    if (mdl.mdlhdr.numhitboxsets > 0)
    {
        Stream.seek(mdl.mdlhdr.hitboxsetindex);
        OutStream.seek(v53Hdr.hitboxsetindex);
        UI::Progress.SubTask.Begin("Updating Hitbox Sets");
        for (int i = 0; i < mdl.mdlhdr.numhitboxsets; i++)
        {
            OutStream.Write(mdl.hitboxsets[i]);
            Logger::Notice("Updated hitbox set %d of %d\n", i + 1, mdl.mdlhdr.numhitboxsets);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numhitboxsets);
        }
        UI::Progress.SubTask.End();
    }

    if (mdl.hitboxes.size() > 0)
    {
        UI::Progress.SubTask.Begin("Updating Hitboxes");
        for (int j = 0; j < hitboxes.size(); j++)
        {
            mstudiobbox_t_v53 hitbox = hitboxes[j];
            OutStream.Write(hitbox);
            Logger::Notice("Updated hitboxes %d of %d\n", j + 1, hitboxes.size());
        }
        UI::Progress.SubTask.End();
    }

    if (mdl.mdlhdr.numbones > 0)
    {
        Stream.seek(mdl.mdlhdr.bonetablebynameindex);
        OutStream.seek(v53Hdr.bonetablebynameindex);
        UI::Progress.SubTask.Begin("Converting BoneTable");
        for (int i = 0; i < mdl.mdlhdr.numbones; i++)
        {
            OutStream.Write(mdl.bonenametable);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numbones);
        }
        UI::Progress.SubTask.End();
    }
    if (mdl.mdlhdr.numlocalanim > 0)
    {
        OutStream.seek(v53Hdr.localanimindex);
        UI::Progress.SubTask.Begin("Converting Animations");
        int ikRuleNum = 0;
        //std::vector<int> ikRuleStairsPerAnim = mdl.v53IkRuleStairsPerAnim();
        for (int i = 0; i < mdl.mdlhdr.numlocalanim; i++)
        {
            //bool isDelta = false;
            //for (int k = 0; k < deltaAnims.size(); k++)
            //{
            //    if (i == deltaAnims[k])
            //    {
            //        isDelta = true;
            //        break;
            //    }
            //}
            mstudioanimdesc_t_v53 animDesc = animdescs[i];
            OutStream.Write(animDesc);
            Logger::Notice("Converted animation %d of %d\n", i + 1, mdl.mdlhdr.numlocalanim);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalanim);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished animations\n");


        int boneHdrNum = 0;
        int secBoneHdrNum = 0;
        int secIdxNum = 0;
        int secNumber = 0;
        int animDescPadding = mdl.mdlhdr.localanimindex + animdescs[0].animindex - ( mdl.mdlhdr.localanimindex + (92 * mdl.mdlhdr.numlocalanim) );

        OutStream.Padding(animDescPadding);
        for (int i = 0; i < mdl.mdlhdr.numlocalanim; i++)
        {

            //bool isDelta = false;
            //for (int k = 0; k < deltaAnims.size(); k++)
            //{
            //    if (i == deltaAnims[k])
            //    {
            //        isDelta = true;
            //        break;
            //    }
            //}

            mstudioanimdesc_t_v53 animDesc = animdescs[i];
            int startPos = v53Hdr.localanimindex + 92 * i;
            OutStream.seek(startPos + animdescs[i].animindex);

            if (animDesc.sectionindex == 0)
            {
                OutStream.seek(startPos + animdescs[i].animindex);

                for (int j = boneHdrNum; j < anims.size(); j++)
                {
                    OutStream.Write(anims[j]);
                    OutStream.Write(anims[j].animdata);
                    boneHdrNum++;
                    if (anims[j].nextoffset == 0)
                    {
                        break;
                    }
                }
            }

            if (animDesc.sectionindex > 0) //Gosh do I hate how this is setup. - Liberty //Edit: Not anymore. Could be better but I leave you with this. - Liberty
            {
                OutStream.seek(startPos + animDesc.sectionindex);

                secHdrBytesSecAdd[0] = 0;
                int num = (animDesc.numframes / animDesc.sectionframes) + 2;

                for (int j = 0; j < num; j++)
                {

                    OutStream.Write(sectionIndexes[secIdxNum]);
                    secIdxNum++;
                }

                for (int j = 0; j < num; j++)
                {
                    int off = sectionIndexes[secNumber].sectionoffsets;

                    OutStream.seek(startPos + off);
                    for (int k = secBoneHdrNum; k < sections.size(); k++)
                    {
                        OutStream.Write(sections[k]);
                        OutStream.Write(sections[k].animdata);
                        secBoneHdrNum++;
                        if (sections[k].nextoffset == 0)
                        {
                            break;
                        }
                    }
                    secNumber++;
                }

            }

            if (animDesc.numikrules > 0)
            {
                OutStream.seek(startPos + animdescs[i].ikruleindex - 32); //Issue with the 32 byte filler. - Liberty
                fillerWrite(&OutStream, 32);
                for (int j = 0; j < animDesc.numikrules; j++)
                {
                    OutStream.Write(ikRules[ikRuleNum]);
                    ikRuleNum++;
                }
                int compressedErrorNum = 0;
                if (mdl.compressedikerrors.size() > 0)
                {
                    OutStream.seek(startPos + animdescs[i].ikruleindex + 140 * animDesc.numikrules);
                    int animStartPos = mdl.mdlhdr.localanimindex + 100 * i;
                    for (int j = 0; j < mdl.compressedikerrors.size(); j++)
                    {
                        OutStream.Write(mdl.compressedikerrors[compressedErrorNum]);
                        OutStream.Write(mdl.compressedikerrors[compressedErrorNum].animdata);
                        compressedErrorNum++;

                    }
                }

            }
        }
    }
    ////sequences
    if (mdl.mdlhdr.numlocalseq > 0)
    {
        int weightListNum = 0;
        int blendNum = 0;
        int poseKeyNum = 0;
        int autoLayerNum = 0;
        int eventNum = 0;
        int numOfActMods = 0;
        int numOfActMods4 = 0;
        int actModNum = 0;


        OutStream.seek(v53Hdr.localseqindex);
        UI::Progress.SubTask.Begin("Converting Sequences");
        for (int i = 0; i < mdl.mdlhdr.numlocalseq; i++) 
        {
            mstudioseqdesc_t_v53 seqDesc = seqdescs[i];
            if (seqDesc.animindexindex > 0)
            {
                for (int j = 0; j < seqDesc.numblends; j++)
                {
                    int anim = mdl.blends[blendNum].blends[j];
                    if (animdescs[anim].numframes == 1)
                    {
                        seqDesc.flags += 0x80000;
                        break;
                    }
                }
                blendNum++;
            }


            OutStream.Write(seqDesc);
            Logger::Notice("Converted sequence %d of %d\n", i + 1, mdl.mdlhdr.numlocalseq);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalseq);
        }
        blendNum = 0;
        for (int i = 0; i < mdl.mdlhdr.numlocalseq; i++)
        {
            mstudioseqdesc_t_v53 seqDesc = seqdescs[i];
            int strPos = mdl.mdlhdr.localseqindex + 212 * i;
            int outPos = v53Hdr.localseqindex + 232 * i;

            Stream.seek(strPos);
            OutStream.seek(outPos);

            if (seqDesc.weightlistindex > 0)
            {
                seqweightlist_t_v49 weightList = mdl.seqweightlist[weightListNum];
                OutStream.seek(outPos + seqDesc.weightlistindex);
                OutStream.Write(weightList);
                weightListNum++;
            }

            if (seqDesc.animindexindex > 0)
            {
                OutStream.seek(outPos + seqDesc.animindexindex);
                OutStream.Write(mdl.blends[blendNum]);
                blendNum++;
            }

            if (seqDesc.posekeyindex > 0)
            {
                int posekeySize = seqdescs[i].groupsize[0] + seqdescs[i].groupsize[1];
                OutStream.seek(outPos + seqDesc.posekeyindex);
                OutStream.Write(mdl.posekeys[poseKeyNum], posekeySize);
                poseKeyNum++;
            }

            if (seqDesc.numautolayers > 0)
            {
                OutStream.seek(outPos + seqDesc.autolayerindex);
                for (int j = 0; j < seqDesc.numautolayers; j++)
                {
                    OutStream.Write(mdl.autolayers[autoLayerNum]);
                    autoLayerNum++;
                }
            }
            if (seqDesc.numevents > 0)
            {
                OutStream.seek(outPos + seqDesc.eventindex);
                for (int j = 0; j < seqDesc.numevents; j++)
                {
                    OutStream.Write(mdl.events[eventNum]);
                    eventNum++;
                }
            }

            if (seqDesc.numactivitymodifiers > 0)
            {
                OutStream.seek(outPos + seqDesc.activitymodifierindex);
                for (int j = 0; j < seqDesc.numactivitymodifiers; j++)
                {
                    OutStream.seek(outPos + seqDesc.activitymodifierindex + 8 * j);
                    OutStream.Write(mdl.activitymodifiers[actModNum]);
                    actModNum++;
                }
            }

        }

        if (mdl.mdlhdr.numlocalnodes > 0)
        {
            UI::Progress.SubTask.Begin("Updating Local Node Names");
            OutStream.seek(v53Hdr.localnodenameindex);
            for (int i = 0; i < mdl.mdlhdr.numlocalnodes; i++)
            {
                OutStream.Write(mdl.nodenames[i]);
                Logger::Notice("Updated node name %d of %d\n", i + 1, mdl.mdlhdr.numlocalnodes);
                UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalnodes);
            }
            OutStream.seek(v53Hdr.localnodeindex);
            for (int i = 0; i < mdl.mdlhdr.numlocalnodes; i++)
            {
                OutStream.Write(mdl.nodes[i]);
                Logger::Notice("Updated node %d of %d\n", i + 1, mdl.mdlhdr.numlocalnodes);
                UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalnodes);
            }
        }


        UI::Progress.SubTask.End();
        Logger::Info("Finished sequences\n");
    }
    if (mdl.mdlhdr.numbodyparts > 0)
    {
        UI::Progress.SubTask.Begin("Converting Body Parts");
        OutStream.seek(v53Hdr.bodypartindex);

        for (int i = 0; i < mdl.mdlhdr.numbodyparts; i++) {
            //16 bytes
            OutStream.seek(v53Hdr.bodypartindex + 16 * i);
            OutStream.Write(mdl.bodyparts[i]);
            Logger::Notice("Converted body part %d of %d\n", i + 1, mdl.mdlhdr.numbodyparts);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numbodyparts);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished body parts\n");
    }


    if (mdl.models.size() > 0)
    {
        UI::Progress.SubTask.Begin("Updating Model Structs");
        for (int i = 0; i < mdl.models.size(); i++)
        {
            OutStream.seek((v53Hdr.bodypartindex + 16 * v53Hdr.numbodyparts) + 148 * i);
            OutStream.Write(mdl.models[i]);
            Logger::Notice("Updated model struct %d of %d\n", i + 1, mdl.models.size());
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished model structs\n");
    }



    if (ikChains.size() > 0)
    {
        UI::Progress.SubTask.Begin("Converting IkChains");
        for (int i = 0; i < mdl.mdlhdr.numikchains; i++)
        {
            OutStream.seek(v53Hdr.ikchainindex + (32 * i));
            OutStream.Write(ikChains[i]);
            Logger::Notice("Converting IkChain %d of %d\n", i + 1, mdl.mdlhdr.numikchains);
        }

        for (int i = 0; i < mdl.iklinks.size(); i++)
        {
            OutStream.seek((v53Hdr.ikchainindex + (32 * v53Hdr.numikchains)) + 28 * i);
            OutStream.Write(mdl.iklinks[i]);
        }



        UI::Progress.SubTask.End();
        Logger::Info("Finished Converting IkChains\n");
    }

    if (mdl.meshes.size() > 0)
    {
        UI::Progress.SubTask.Begin("Updating Mesh Structs");
        for (int i = 0; i < mdl.meshes.size(); i++)
        {
            OutStream.seek((v53Hdr.ikchainindex + (20 * mdl.mdlhdr.numlocalposeparameters) + (32 * v53Hdr.numikchains)) + (28 * mdl.iklinks.size()) + 116 * i);
            OutStream.Write(mdl.meshes[i]);
            Logger::Notice("Updated mesh struct %d of %d\n", i + 1, mdl.meshes.size());
        }

        UI::Progress.SubTask.End();
        Logger::Info("Finished mesh structs\n");
    }

    if (mdl.mdlhdr.numlocalposeparameters > 0)
    {
        UI::Progress.SubTask.Begin("Updating PoseParameters");
        OutStream.seek(v53Hdr.localposeparamindex);
        for (int i = 0; i < mdl.mdlhdr.numlocalposeparameters; i++)
        {
            OutStream.Write(mdl.poseparamdescs[i]);
            Logger::Notice("Updating PoseParameter %d of %d\n", i + 1, mdl.mdlhdr.numlocalposeparameters);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished Updating PoseParameters\n");
    }

    if (mdl.mdlhdr.numincludemodels > 0)
    {
        UI::Progress.SubTask.Begin("Converting Included Models");

        OutStream.seek(v53Hdr.includemodelindex);
        for (int i = 0; i < mdl.mdlhdr.numincludemodels; i++)
        {
            //8 bytes
            OutStream.Write(mdl.includedmodels[i]);

            Logger::Notice("Converted included model %d of %d\n", i + 1, mdl.mdlhdr.numincludemodels);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numincludemodels);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished included models\n");
    }
    //RUI CODE 

    if (info.rui.has_value()) {
        BinaryReader RUIStream = BinaryReader(info.rui.value().c_str());
        if (!RUIStream.Stream.good()) {
            Logger::Error("Model's RUI file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.rui.value().c_str());
            return 1;
        }
        printf("writing rui at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying RUI Data");
        OutStream.seek(v53Hdr.includemodelindex + 8 * v53Hdr.numincludemodels);
        int padding = 0;
        mstudiorruiheader_t ruiHeader;
        for (int i = 0; i < v53Hdr.numruimeshes; i++)
        {
            RUIStream.Read(&ruiHeader);
            if (i == 0)
            {
                padding = ruiHeader.ruimeshindex - 8 * v53Hdr.numruimeshes;
                //Logger::Info("Padding:  %d\n", padding);
            }
            ruiHeader.ruimeshindex -= padding;
            ruiHeader.ruimeshindex += mdl.ruiPadding;

            OutStream.Write(ruiHeader);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)RUIStream.size);
        }
        OutStream.Padding(mdl.ruiPadding);
        for (int i = 0; i < padding; i++)
        {
            byte tmp;
            RUIStream.Read(&tmp);
        }
        for (int i = 0; i < RUIStream.size - (8 * v53Hdr.numruimeshes + padding); i++)
        {
            byte tmp;
            RUIStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)RUIStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }
    //
    //if (ruiMeshes.size() > 0 && !info.aabb.has_value() && info.str.has_value())
    //{
    //    for (int i = 0; i < ruiMeshes.size(); i++)
    //    {
    //        OutStream.Write(0);
    //        OutStream.Write( (int)( 8 * ( ruiMeshes.size() - i ) + bytesAddedPerRuiMesh[i]));
    //        double sizeCheck = GetRuiBytesAddedIdv(ruiMeshes, i) / 16.0;
    //        int idk = 0;
    //        Logger::Info("sizeCheck: %d, rui: %d\n", GetRuiBytesAddedIdv(ruiMeshes, i), i);
    //        if (!IsWhole(sizeCheck))
    //        {
    //            Logger::Info("sizeCheck: %d, rui: %d\n", sizeCheck, i);
    //            double sizeCheck2 = 16 * ( (int)sizeCheck + 1);
    //            idk = sizeCheck2 - GetRuiBytesAddedIdv(ruiMeshes, i);
    //        }
    //        
    //        int parentIdxAdd = 2 * ruiMeshes[i].numparents;
    //        int facesIdxAdd = 32 * ruiMeshes[i].numfaces;
    //        int verticesIdxAdd = 16 * ruiMeshes[i].numvertices;
    //        int verticesIdxAdd2 = 6 * ruiMeshes[i].numfaces;
    //        int meshNameIdxAdd = ruiMeshes[i].szruimeshname.size() + idk;
    //        int unk = 0;
    //        std::byte null = (std::byte)0;
    //        Logger::Info("idk: %d, rui: %d\n", idk, i);
    //        OutStream.Write(ruiMeshes[i].numparents);
    //        OutStream.Write(ruiMeshes[i].numvertices);
    //        OutStream.Write(ruiMeshes[i].numfaces);
    //        OutStream.Write((int)(32 + meshNameIdxAdd));
    //        OutStream.Write((int)(32 + meshNameIdxAdd + verticesIdxAdd2 + parentIdxAdd));
    //        OutStream.Write((int)(32 + meshNameIdxAdd + parentIdxAdd));
    //        OutStream.Write((int)(32 + meshNameIdxAdd + parentIdxAdd + verticesIdxAdd + verticesIdxAdd2));
    //        OutStream.Write(unk);
    //        OutStream.write(ruiMeshes[i].szruimeshname.c_str(), ruiMeshes[i].szruimeshname.size());
    //        for (int j = 0; j < idk; j++) OutStream.Write(null);
    //
    //        for (int j = 0; j < ruiMeshes[i].numparents; j++)
    //        {
    //            OutStream.Write(ruiMeshes[i].parent[j]);
    //        }
    //
    //        for (int j = 0; j < ruiMeshes[i].numfaces; j++)
    //        {
    //            OutStream.Write(ruiMeshes[i].vertexmap[j].vertstartid);
    //            OutStream.Write(ruiMeshes[i].vertexmap[j].vertendid);
    //            OutStream.Write(ruiMeshes[i].vertexmap[j].vertextraid);
    //        }
    //
    //        for (int j = 0; j < ruiMeshes[i].numvertices; j++)
    //        {
    //            OutStream.Write(ruiMeshes[i].vertex[j].parent);
    //            OutStream.Write(ruiMeshes[i].vertex[j].vertexpos);
    //        }
    //
    //        for (int j = 0; j < ruiMeshes[i].numfaces; j++)
    //        {
    //            OutStream.Write(ruiMeshes[i].facedata[j].faceuvmin.x);
    //            OutStream.Write(ruiMeshes[i].facedata[j].faceuvmin.y);
    //            OutStream.Write(ruiMeshes[i].facedata[j].faceuvmax.x);
    //            OutStream.Write(ruiMeshes[i].facedata[j].faceuvmax.y);
    //            OutStream.Write(ruiMeshes[i].facedata[j].facescalemin.x);
    //            OutStream.Write(ruiMeshes[i].facedata[j].facescalemin.y);
    //            OutStream.Write(ruiMeshes[i].facedata[j].facescalemax.x);
    //            OutStream.Write(ruiMeshes[i].facedata[j].facescalemax.y);
    //        }
    //    }
    //}


#pragma region texture conversion
    AddInt32(&OutStream, 0, 3);  //filler. Edit: I don't remember why. - Liberty
    OutStream.seek(v53Hdr.textureindex);
    UI::Progress.SubTask.Begin("Converting Textures");
    for (int i = 0; i < mdl.mdlhdr.numtextures; i++)
    {

        OutStream.Write(textures[i]);

        Logger::Notice("Converted texture %d of %d\n", i + 1, mdl.mdlhdr.numtextures);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numtextures);
    }
    UI::Progress.SubTask.End();
    Logger::Info("Finished textures\n");
#pragma endregion

    UI::Progress.SubTask.Begin("Converting Cd Textures");
    OutStream.seek(v53Hdr.cdtextureindex);
    for (int i = 0; i < mdl.mdlhdr.numcdtextures; i++)
    {
        OutStream.Write(mdl.cdtextures[i]);
        Logger::Notice("Converted texture %d of %d\n", i + 1, mdl.mdlhdr.numcdtextures);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numcdtextures);
    }
    UI::Progress.SubTask.End();
    Logger::Info("Finished cd textures\n");


    UI::Progress.SubTask.Begin("Converting Skin Groups");
    OutStream.seek(v53Hdr.skinindex);
    for (int i = 0; i < mdl.mdlhdr.numskinfamilies; i++)
    {

        OutStream.Write(mdl.skingroups[i], mdl.mdlhdr.numskinref);
        Logger::Notice("Converted skin group %d of %d\n", i + 1, mdl.mdlhdr.numskinfamilies);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numskinfamilies);
    }
    UI::Progress.SubTask.End();
    Logger::Info("Finished skin groups\n");

    if (mdl.mdlhdr.keyvaluesize > 0)
    {
        OutStream.seek(v53Hdr.keyvalueindex);
        OutStream.Write(mdl.keyvalues);
    }

    UI::Progress.SubTask.Begin("Converting Src Bone Transforms");
    OutStream.seek(v53Hdr.srcbonetransformindex);
    for (int i = 0; i < mdl.mdlsubhdr.numsrcbonetransform; i++)
    {
        OutStream.Write(mdl.srcbonetransforms[i]);
        Logger::Notice("Converting Src Bone Transform %d of %d\n", i + 1, mdl.mdlsubhdr.numsrcbonetransform);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlsubhdr.numsrcbonetransform);
    }
    UI::Progress.SubTask.End();
    Logger::Info("Finished src bone transforms\n");

    OutStream.seek(v53Hdr.linearboneindex);
    for (int i = 0; i < mdl.mdlhdr.numbones; i++)
    {
        for (int j = 0; j < ikChainBones.size(); j++)
        {
            bool hasIkFlag = mdl.linearbonedata.flags[i] & 0x20;
            if (i == ikChainBones[j] && !hasIkFlag)
            {
                mdl.linearbonedata.flags[i] += 0x20;
            }
        }
        mdl.linearbonedata.posScale[i] = { 0,0,0 };
    }
    OutStream.Write(mdl.linearbone);
    OutStream.Write(linearBoneData, mdl.mdlhdr.numbones);
    OutStream.Write(mdl.pertriheader);
    if (mdl.mdlsubhdr.numaabbnodes > 0)
    {
        for (int i = 0; i < mdl.mdlsubhdr.numaabbnodes; i++)
        {
            OutStream.Write(mdl.aabbnodes[i]);
        }
    }

    if (mdl.mdlsubhdr.numaabbnodes > 0)
    {
        for (int i = 0; i < mdl.mdlsubhdr.numaabbleaves; i++)
        {
            OutStream.Write(mdl.aabbleaves[i]);
        }
    }

    if (mdl.mdlsubhdr.numaabbverts > 0)
    {
        OutStream.Write(mdl.aabbverts);
    }
    OutStream.seek(OutStream.Position() + 1);
    OutStream.Write(mdl.stringtable); //How the fuck is this working? - Liberty Edit: Fixed it for v49. - Liberty

#pragma region rest of the file
    int pos = Stream.Position();
    //UI::Progress.SubTask.Begin("Copying Misc Data");
    //for (int i = 0; i < Stream.size - pos; i++) {
    //    byte tmp;
    //    Stream.Read(&tmp);
    //    OutStream.Write(tmp);
    //    UI::Progress.SubTask.Update((i + 1.0f) / (float)(Stream.size - pos));
    //}
    //UI::Progress.SubTask.End();

    if (OutStream.Position() < Stream.size + allBytesAdded) {
        Logger::Critical("Not enough of the file was written\n");
        Logger::Critical("(%d vs %d)\n", OutStream.Position(), Stream.size + allBytesAdded);
        return 1;
    }

    Logger::Notice("Wrote Rest of File\n");
    if (OutStream.Position() < Stream.Position() + allBytesAdded) {
        Logger::Critical("Relative cursors are desynced\n");
        Logger::Critical("(%d vs %d)\n", OutStream.Position(), Stream.Position() + allBytesAdded);
        return 1;
    }

    if (OutStream.Position() < Stream.Position() + allBytesAdded) {
        Logger::Critical("Relative cursors are desynced\n");
        Logger::Critical("(%d vs %d)\n", OutStream.Position(), Stream.Position() + allBytesAdded);
        return 1;
    }


    if (info.phy.has_value()) {
        BinaryReader PhyStream = BinaryReader(info.phy.value().c_str());
        if (!PhyStream.Stream.good()) {
            Logger::Error("Model's phy file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.phy.value().c_str());
            return 1;
        }
        printf("writing phy at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Physics Data");
        for (int i = 0; i < PhyStream.size; i++) {
            byte tmp;
            PhyStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)PhyStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    if (info.pfb.has_value()) {
        BinaryReader PhyBStream = BinaryReader(info.pfb.value().c_str());
        if (!PhyBStream.Stream.good()) {
            Logger::Error("Model's rphy file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.pfb.value().c_str());
            return 1;
        }
        printf("writing rphy at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Respawn Physics Data");
        for (int i = 0; i < PhyBStream.size; i++) {
            byte tmp;
            PhyBStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)PhyBStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }


    if (info.vtx.has_value()) {
        BinaryReader VtxStream = BinaryReader(info.vtx.value().c_str());
        if (!VtxStream.Stream.good()) {
            Logger::Error("Model's vtx file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.vtx.value().c_str());
            return 1;
        }
        Logger::Info("writing vtx at [%d]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Mesh Data");
        for (int i = 0; i < VtxStream.size; i++) {
            byte tmp;
            VtxStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)VtxStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    if (info.vvd.has_value()) {
        BinaryReader VvdStream = BinaryReader(info.vvd.value().c_str());
        if (!VvdStream.Stream.good()) {
            Logger::Error("Model's vvd file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.vvd.value().c_str());
            return 1;
        }
        Logger::Info("writing vvd at [%d]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Vertex Data");
        for (int i = 0; i < VvdStream.size; i++) {
            byte tmp;
            VvdStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)VvdStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    if (info.vvc.has_value()) {
        BinaryReader VvcStream = BinaryReader(info.vvc.value().c_str());
        if (!VvcStream.Stream.good()) {
            Logger::Error("Model's vvc file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.vvc.value().c_str());
            return 1;
        }
        Logger::Info("writing vvc at [%d]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Vertex Color Data");
        for (int i = 0; i < VvcStream.size; i++) {
            byte tmp;
            VvcStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)VvcStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    OutStream.Stream.close();
    Stream.Stream.close();

    Logger::Info("Finished!\n");
    UI::Progress.MainTask.End();
    return 0;
}

int ConvertV49(FileInfo info)
{
    Logger::Debug("Conversion started\n");
    UI::Progress.MainTask.Begin("Converting");

    BinaryReader Stream = BinaryReader(info.mdl.value().c_str());
    BinaryWriter OutStream = BinaryWriter(info.out.value().c_str());
    JsonTest jsonController{}; //Made it class because why not. -Liberty
    if (OutStream.Position() > 0) OutStream.seek(0);
    if (Stream.Position() > 0) Stream.seek(0);

    MDL::v49Mdl mdl = mdl._v49Mdl(&Stream, false);
    mdl.SetMdlInts();
    studiohdr_t_v53 v53Hdr = mdl.ConvertHeader(info);
    OutStream.Write(v53Hdr);
    mdl.UpdateMdl();

    std::vector<int> bytesAddedPerRuiMesh;
    std::vector<mstudioruimesh_t> ruiMeshes;

    std::vector<int> hdrBytesAnimDescAdd = mdl.v53GetAnimHdrBytesAdded(true);
    std::vector<int> secHdrBytesAnimDescAdd = mdl.v53GetSecHdrBytesAdded(true);
    int animByteAddedTotal = mdl.v53GetTotalAnimHdrBytesAdded();
    int animSecByteAddedTotal = mdl.v53GetTotalSecHdrBytesAdded();

    std::vector<int> secHdrBytesSecAdd = mdl.v53GetSecBytesAdded(true);
    std::vector<int> ikChainBones = mdl.GetIKBones();
    //std::vector<int> deltaAnims = GetDeltaAnims(&Stream, Initial_Header, false);

    int animBytesAdded = animByteAddedTotal + animSecByteAddedTotal;
    int bytesAddedToRuiMesh = 0;
    int bytesAddedToIkRules = -12 * mdl.iklinks.size();
    int bytesAddedToHeader = 52;
    int bytesAddedToBones = mdl.mdlhdr.numbones * 28;
    int bytesAddedToAnims = -8 * mdl.mdlhdr.numlocalanim;
    int bytesAddedToAnimData = mdl.mdlhdr.numlocalanim > 0 ? animByteAddedTotal + animSecByteAddedTotal + bytesAddedToIkRules : 0;
    int bytesAddedToSeqs = 20 * mdl.mdlhdr.numlocalseq;
    int bytesAddedToTextures = -20 * mdl.mdlhdr.numtextures;
    int bytesAddedToIkChains = 16 * mdl.mdlhdr.numikchains;
    int bytesAddedToActMods = 4 * mdl.activitymodifiers.size();
    int bytesAddedToLinearBone = -12 * mdl.mdlhdr.numbones;
    int textureFiller = 0;
    int strFiller = 60;
    int allBytesAdded = bytesAddedToHeader + bytesAddedToBones + bytesAddedToAnims + bytesAddedToSeqs + bytesAddedToTextures + bytesAddedToIkChains + bytesAddedToAnimData + bytesAddedToActMods + textureFiller + bytesAddedToRuiMesh + bytesAddedToLinearBone;

    jsonController.ReadExtraStructs(info);
    std::vector<mstudioanim_t_v53> anims = mdl.ConvertAnims();
    std::vector<mstudioanim_t_v53> sections = mdl.ConvertSections();
    std::vector<mstudiobone_t_v53> bones = mdl.BoneConversion();
    std::vector<mstudiobbox_t_v53> hitboxes = mdl.HitboxConversion();
    std::vector<mstudioanimdesc_t_v53> animdescs = mdl.AnimDescConversion();
    std::vector<sectionindexes_t_v53> sectionIndexes = mdl.ConvertSectionIndexes();
    std::vector<mstudioikrule_t_v53> ikRules = mdl.IkRuleConversion();
    std::vector<mstudioseqdesc_t_v53> seqdescs = mdl.SeqDescConversion();
    std::vector<mstudioikchain_t_v53> ikChains = mdl.IkChainConversion();
    std::vector<mstudiotexture_t_v53> textures = mdl.ConvertTextures();
    std::vector<mstudioactivitymodifier_t_v53> activityModifiers = mdl.ActivityModifierConversion();
    mstudiolinearbonedata_t_v53 linearBoneData = mdl.ConvertLinearBoneData();

    OutStream.seek(v53Hdr.boneindex);
    UI::Progress.SubTask.Begin("Converting Bones");
    for (int i = 0; i < mdl.mdlhdr.numbones; i++)
    {
        mstudiobone_t_v53 bone = bones[i];

        for (int j = 0; j < ikChainBones.size(); j++)
        {
            if (i == ikChainBones[j]) bone.flags += 0x20;
        }
        bone.unkid = -1;
        OutStream.Write(bone);

        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numbones);
    }
    if (mdl.jigglebones.size() > 0)
    {
        for (int i = 0; i < mdl.jigglebones.size(); i++)
        {
            mstudiojigglebone_t_v49 jiggleBone = mdl.jigglebones[i];

            OutStream.Write(jiggleBone);
        }
    }
    UI::Progress.SubTask.End();
    OutStream.seek(v53Hdr.localattachmentindex);
    UI::Progress.SubTask.Begin("Converting Attachments");
    for (int i = 0; i < v53Hdr.numlocalattachments; i++)
    {
        OutStream.Write(mdl.attachments[i]);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalattachments);
    }
    UI::Progress.SubTask.End();

    //hitboxsets

    Stream.seek(mdl.mdlhdr.hitboxsetindex);
    OutStream.seek(v53Hdr.hitboxsetindex);
    UI::Progress.SubTask.Begin("Updating Hitbox Sets");
    for (int i = 0; i < mdl.mdlhdr.numhitboxsets; i++)
    {
        OutStream.Write(mdl.hitboxsets[i]);
        Logger::Notice("Updated hitbox set %d of %d\n", i + 1, mdl.mdlhdr.numhitboxsets);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numhitboxsets);
    }
    UI::Progress.SubTask.End();

    if (mdl.hitboxes.size() > 0)
    {
        UI::Progress.SubTask.Begin("Updating Hitboxes");
        for (int j = 0; j < hitboxes.size(); j++)
        {
            mstudiobbox_t_v53 hitbox = hitboxes[j];
            OutStream.Write(hitbox);
            Logger::Notice("Updated hitboxes %d of %d\n", j + 1, hitboxes.size());
        }
        UI::Progress.SubTask.End();
    }

    Stream.seek(mdl.mdlhdr.bonetablebynameindex);
    OutStream.seek(v53Hdr.bonetablebynameindex);
    UI::Progress.SubTask.Begin("Converting BoneTable");
    for (int i = 0; i < mdl.mdlhdr.numbones; i++)
    {
        OutStream.Write(mdl.bonenametable);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numbones);
    }
    UI::Progress.SubTask.End();

    OutStream.seek(v53Hdr.localanimindex);
    UI::Progress.SubTask.Begin("Converting Animations");
    int ikRuleNum = 0;
    //std::vector<int> ikRuleStairsPerAnim = mdl.v53IkRuleStairsPerAnim();
    for (int i = 0; i < mdl.mdlhdr.numlocalanim; i++)
    {
        //bool isDelta = false;
        //for (int k = 0; k < deltaAnims.size(); k++)
        //{
        //    if (i == deltaAnims[k])
        //    {
        //        isDelta = true;
        //        break;
        //    }
        //}
        mstudioanimdesc_t_v53 animDesc = animdescs[i];
        OutStream.Write(animDesc);
        Logger::Notice("Converted animation %d of %d\n", i + 1, mdl.mdlhdr.numlocalanim);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalanim);
    }
    UI::Progress.SubTask.End();
    Logger::Info("Finished animations\n");

    int boneHdrNum = 0;
    int secBoneHdrNum = 0;
    int secIdxNum = 0;
    int secNumber = 0;

    for (int i = 0; i < mdl.mdlhdr.numlocalanim; i++)
    {

        //bool isDelta = false;
        //for (int k = 0; k < deltaAnims.size(); k++)
        //{
        //    if (i == deltaAnims[k])
        //    {
        //        isDelta = true;
        //        break;
        //    }
        //}

        mstudioanimdesc_t_v53 animDesc = animdescs[i];
        int startPos = v53Hdr.localanimindex + 92 * i;
        OutStream.seek(startPos + animdescs[i].animindex);

        if (animDesc.sectionindex == 0)
        {
            OutStream.seek(startPos + animdescs[i].animindex);

            for (int j = boneHdrNum; j < anims.size(); j++)
            {
                OutStream.Write(anims[j]);
                OutStream.Write(anims[j].animdata);
                boneHdrNum++;
                if (anims[j].nextoffset == 0)
                {
                    break;
                }
            }
        }

        if (animDesc.sectionindex > 0) //Gosh do I hate how this is setup. - Liberty //Edit: Not anymore. Could be better but I leave you with this. - Liberty
        {
            OutStream.seek(startPos + animDesc.sectionindex);

            secHdrBytesSecAdd[0] = 0;
            int num = (animDesc.numframes / animDesc.sectionframes) + 2;

            for (int j = 0; j < num; j++)
            {

                OutStream.Write(sectionIndexes[secIdxNum]);
                secIdxNum++;
            }

            for (int j = 0; j < num; j++)
            {
                int off = sectionIndexes[secNumber].sectionoffsets;

                OutStream.seek(startPos + off);
                for (int k = secBoneHdrNum; k < sections.size(); k++)
                {
                    OutStream.Write(sections[k]);
                    OutStream.Write(sections[k].animdata);
                    secBoneHdrNum++;
                    if (sections[k].nextoffset == 0)
                    {
                        break;
                    }
                }
                secNumber++;
            }

        }

        if (animDesc.numikrules > 0)
        {
            OutStream.seek(startPos + animdescs[i].ikruleindex - 32); //Issue with the 32 byte filler. - Liberty
            fillerWrite(&OutStream, 32);
            for (int j = 0; j < animDesc.numikrules; j++)
            {
                OutStream.Write(ikRules[ikRuleNum]);
                ikRuleNum++;
            }
            int compressedErrorNum = 0;
            if (mdl.compressedikerrors.size() > 0)
            {
                OutStream.seek(startPos + animdescs[i].ikruleindex + 140 * animDesc.numikrules);
                int animStartPos = mdl.mdlhdr.localanimindex + 100 * i;
                for (int j = 0; j < mdl.compressedikerrors.size(); j++)
                {
                    OutStream.Write(mdl.compressedikerrors[compressedErrorNum]);
                    OutStream.Write(mdl.compressedikerrors[compressedErrorNum].animdata);
                    compressedErrorNum++;

                }
            }

        }
    }
    ////sequences
    OutStream.seek(v53Hdr.localseqindex);
    UI::Progress.SubTask.Begin("Converting Sequences");
    for (int i = 0; i < mdl.mdlhdr.numlocalseq; i++) {
        OutStream.Write(seqdescs[i]);
        Logger::Notice("Converted sequence %d of %d\n", i + 1, mdl.mdlhdr.numlocalseq);
        UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalseq);
    }
    int weightListNum = 0;
    int blendNum = 0;
    int poseKeyNum = 0;
    int autoLayerNum = 0;
    int eventNum = 0;
    int numOfActMods = 0;
    int numOfActMods4 = 0;
    int actModNum = 0;

    for (int i = 0; i < mdl.mdlhdr.numlocalseq; i++)
    {
        mstudioseqdesc_t_v53 seqDesc = seqdescs[i];
        int strPos = mdl.mdlhdr.localseqindex + 212 * i;
        int outPos = v53Hdr.localseqindex + 232 * i;

        Stream.seek(strPos);
        OutStream.seek(outPos);

        if (seqDesc.posekeyindex > 0)
        {
            int posekeySize = seqdescs[i].groupsize[0] + seqdescs[i].groupsize[1];
            OutStream.seek(outPos + seqDesc.posekeyindex);
            OutStream.Write(mdl.posekeys[poseKeyNum], posekeySize);
            poseKeyNum++;
        }

        if (seqDesc.weightlistindex > 0)
        {
            seqweightlist_t_v49 weightList = mdl.seqweightlist[weightListNum];
            OutStream.seek(outPos + seqDesc.weightlistindex);
            OutStream.Write(weightList);
            weightListNum++;
        }

        if (seqDesc.animindexindex > 0)
        {
            OutStream.seek(outPos + seqDesc.animindexindex);
            OutStream.Write(mdl.blends[blendNum]);
            blendNum++;
        }

        if (seqDesc.numautolayers > 0)
        {
            OutStream.seek(outPos + seqDesc.autolayerindex);
            for (int j = 0; j < seqDesc.numautolayers; j++)
            {
                OutStream.Write(mdl.autolayers[autoLayerNum]);
                autoLayerNum++;
            }
        }
        if (seqDesc.numevents > 0)
        {
            OutStream.seek(outPos + seqDesc.eventindex);
            for (int j = 0; j < seqDesc.numevents; j++)
            {
                OutStream.Write(mdl.events[eventNum]);
                eventNum++;
            }
        }

        if (seqDesc.numactivitymodifiers > 0)
        {
            OutStream.seek(outPos + seqDesc.activitymodifierindex);
            for (int j = 0; j < seqDesc.numactivitymodifiers; j++)
            {
                OutStream.seek(outPos + seqDesc.activitymodifierindex + 8 * j);
                if (seqDesc.numactivitymodifiers == 2 && j == 1 && activityModifiers[actModNum].szname.c_str() != "air" && activityModifiers[actModNum - 1].szname.c_str() != "sliding" || seqDesc.numactivitymodifiers == 2 && j == 1 && activityModifiers[actModNum].szname != "sliding" && activityModifiers[actModNum - 1].szname != "air") activityModifiers[actModNum].unk == 1;
                OutStream.Write(activityModifiers[actModNum]);
                actModNum++;
            }
        }

    }

    if (mdl.mdlhdr.numlocalnodes > 0)
    {
        UI::Progress.SubTask.Begin("Updating Local Node Names");
        OutStream.seek(v53Hdr.localnodenameindex);
        for (int i = 0; i < mdl.mdlhdr.numlocalnodes; i++)
        {
            OutStream.Write(mdl.nodenames[i]);
            Logger::Notice("Updated node name %d of %d\n", i + 1, mdl.mdlhdr.numlocalnodes);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalnodes);
        }
        OutStream.seek(v53Hdr.localnodeindex);
        for (int i = 0; i < mdl.mdlhdr.numlocalnodes; i++)
        {
            OutStream.Write(mdl.nodes[i]);
            Logger::Notice("Updated node %d of %d\n", i + 1, mdl.mdlhdr.numlocalnodes);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numlocalnodes);
        }
    }


    UI::Progress.SubTask.End();
    Logger::Info("Finished sequences\n");

    if (mdl.mdlhdr.numbodyparts > 0)
    {
        UI::Progress.SubTask.Begin("Converting Body Parts");
        OutStream.seek(v53Hdr.bodypartindex);

        for (int i = 0; i < mdl.mdlhdr.numbodyparts; i++) {
            //16 bytes
            OutStream.seek(v53Hdr.bodypartindex + 16 * i);
            OutStream.Write(mdl.bodyparts[i]);
            Logger::Notice("Converted body part %d of %d\n", i + 1, mdl.mdlhdr.numbodyparts);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numbodyparts);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished body parts\n");
    }


    if (mdl.models.size() > 0)
    {
        UI::Progress.SubTask.Begin("Updating Model Structs");
        for (int i = 0; i < mdl.models.size(); i++)
        {
            OutStream.seek((v53Hdr.bodypartindex + 16 * v53Hdr.numbodyparts) + 148 * i);
            OutStream.Write(mdl.models[i]);
            Logger::Notice("Updated model struct %d of %d\n", i + 1, mdl.models.size());
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished model structs\n");
    }



    if (ikChains.size() > 0)
    {
        UI::Progress.SubTask.Begin("Converting IkChains");
        for (int i = 0; i < mdl.mdlhdr.numikchains; i++)
        {
            OutStream.seek(v53Hdr.ikchainindex + (32 * i));
            OutStream.Write(ikChains[i]);
            Logger::Notice("Converting IkChain %d of %d\n", i + 1, mdl.mdlhdr.numikchains);
        }

        for (int i = 0; i < mdl.iklinks.size(); i++)
        {
            OutStream.seek((v53Hdr.ikchainindex + (32 * v53Hdr.numikchains)) + 28 * i);
            OutStream.Write(mdl.iklinks[i]);
        }



        UI::Progress.SubTask.End();
        Logger::Info("Finished Converting IkChains\n");
    }

    if (mdl.meshes.size() > 0)
    {
        UI::Progress.SubTask.Begin("Updating Mesh Structs");
        for (int i = 0; i < mdl.meshes.size(); i++)
        {
            OutStream.seek((v53Hdr.ikchainindex + (20 * mdl.mdlhdr.numlocalposeparameters) + (32 * v53Hdr.numikchains)) + (28 * mdl.iklinks.size()) + 116 * i);
            OutStream.Write(mdl.meshes[i]);
            Logger::Notice("Updated mesh struct %d of %d\n", i + 1, mdl.meshes.size());
        }

        UI::Progress.SubTask.End();
        Logger::Info("Finished mesh structs\n");
    }

    if (mdl.mdlhdr.numlocalposeparameters > 0)
    {
        UI::Progress.SubTask.Begin("Updating PoseParameters");
        OutStream.seek(v53Hdr.localposeparamindex);
        for (int i = 0; i < mdl.mdlhdr.numlocalposeparameters; i++)
        {
            OutStream.Write(mdl.poseparamdescs[i]);
            Logger::Notice("Updating PoseParameter %d of %d\n", i + 1, mdl.mdlhdr.numlocalposeparameters);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished Updating PoseParameters\n");
    }
    if (mdl.mdlhdr.numincludemodels > 0)
    {
        UI::Progress.SubTask.Begin("Converting Included Models");

        OutStream.seek(v53Hdr.includemodelindex);
        for (int i = 0; i < mdl.mdlhdr.numincludemodels; i++)
        {
            //8 bytes
            OutStream.Write(mdl.includedmodels[i]);

            Logger::Notice("Converted included model %d of %d\n", i + 1, mdl.mdlhdr.numincludemodels);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numincludemodels);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished included models\n");
    }

    //RUI CODE 

    if (info.rui.has_value()) 
    {
        OutStream.seek(v53Hdr.ruimeshindex);
        BinaryReader RUIStream = BinaryReader(info.rui.value().c_str());
        if (!RUIStream.Stream.good()) {
            Logger::Error("Model's RUI file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.rui.value().c_str());
            return 1;
        }
        printf("writing rui at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying RUI Data");

        int padding = 0;
        mstudiorruiheader_t ruiHeader;
        for (int i = 0; i < v53Hdr.numruimeshes; i++)
        {
            RUIStream.Read(&ruiHeader);
            if (i == 0)
            {
                padding = ruiHeader.ruimeshindex - 8 * v53Hdr.numruimeshes;
                //Logger::Info("Padding:  %d\n", padding);
            }
            ruiHeader.ruimeshindex -= padding;
            ruiHeader.ruimeshindex += mdl.ruiPadding;

            OutStream.Write(ruiHeader);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)RUIStream.size);
        }
        OutStream.Padding(mdl.ruiPadding);
        for (int i = 0; i < padding; i++)
        {
            byte tmp;
            RUIStream.Read(&tmp);
        }
        for (int i = 0; i < RUIStream.size - (8 * v53Hdr.numruimeshes + padding); i++)
        {
            byte tmp;
            RUIStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)RUIStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }


#pragma region texture conversion
    if (mdl.mdlhdr.numtextures > 0)
    {
        AddInt32(&OutStream, 0, 3);  //filler. Edit: I don't remember why. - Liberty
        OutStream.seek(v53Hdr.textureindex);
        UI::Progress.SubTask.Begin("Converting Textures");
        for (int i = 0; i < mdl.mdlhdr.numtextures; i++)
        {

            OutStream.Write(textures[i]);

            Logger::Notice("Converted texture %d of %d\n", i + 1, mdl.mdlhdr.numtextures);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numtextures);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished textures\n");
    }
#pragma endregion
    if (mdl.mdlhdr.numcdtextures > 0)
    {
        UI::Progress.SubTask.Begin("Converting Cd Textures");
        OutStream.seek(v53Hdr.cdtextureindex);
        for (int i = 0; i < mdl.mdlhdr.numcdtextures; i++)
        {
            OutStream.Write(mdl.cdtextures[i]);
            Logger::Notice("Converted texture %d of %d\n", i + 1, mdl.mdlhdr.numcdtextures);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numcdtextures);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished cd textures\n");
    }
    if (mdl.mdlhdr.numskinfamilies > 0)
    {
        UI::Progress.SubTask.Begin("Converting Skin Groups");
        OutStream.seek(v53Hdr.skinindex);
        for (int i = 0; i < mdl.mdlhdr.numskinfamilies; i++)
        {

            OutStream.Write(mdl.skingroups[i], mdl.mdlhdr.numskinref);
            Logger::Notice("Converted skin group %d of %d\n", i + 1, mdl.mdlhdr.numskinfamilies);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlhdr.numskinfamilies);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished skin groups\n");
    }

    if (mdl.mdlhdr.keyvaluesize > 0)
    {
        OutStream.seek(v53Hdr.keyvalueindex);
        OutStream.Write(mdl.keyvalues);
    }
    if (mdl.mdlsubhdr.numsrcbonetransform > 0)
    {
        UI::Progress.SubTask.Begin("Converting Src Bone Transforms");
        OutStream.seek(v53Hdr.srcbonetransformindex);
        for (int i = 0; i < mdl.mdlsubhdr.numsrcbonetransform; i++)
        {
            OutStream.Write(mdl.srcbonetransforms[i]);
            Logger::Notice("Converting Src Bone Transform %d of %d\n", i + 1, mdl.mdlsubhdr.numsrcbonetransform);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)mdl.mdlsubhdr.numsrcbonetransform);
        }
        UI::Progress.SubTask.End();
        Logger::Info("Finished src bone transforms\n");
    }
    if (v53Hdr.linearboneindex > 0 || mdl.mdlhdr.numbones > 1)
    {
        OutStream.seek(v53Hdr.linearboneindex);
        for (int i = 0; i < mdl.mdlhdr.numbones; i++)
        {
            for (int j = 0; j < ikChainBones.size(); j++)
            {
                if (i == ikChainBones[j])
                {
                    mdl.linearbonedata.flags[i] += 0x20;
                }
            }
            //mdl.linearbonedata.posScale[i] = { 0,0,0 };
        }
        OutStream.Write(mdl.linearbone);
        OutStream.Write(linearBoneData, mdl.mdlhdr.numbones);
    }

    if (info.aabb.has_value())
    {
        BinaryReader AABBStream = BinaryReader(info.aabb.value().c_str());
        if (!AABBStream.Stream.good()) {
            Logger::Error("Model's AABB Tree file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.aabb.value().c_str());
            return 1;
        }
        printf("writing AABB Tree at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Respawn AABB Tree Data");
        for (int i = 0; i < AABBStream.size; i++) {
            byte tmp;
            AABBStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)AABBStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }
    else
        AddInt32(&OutStream, 0, 15);

    OutStream.seek(v53Hdr.sznameindex);
    OutStream.Write(mdl.stringtable); //How the fuck is this working? - Liberty Edit: Fixed it for v49. - Liberty

#pragma region rest of the file
    int pos = Stream.Position();

    if (OutStream.Position() < Stream.size + allBytesAdded) {
        Logger::Critical("Not enough of the file was written\n");
        Logger::Critical("(%d vs %d)\n", OutStream.Position(), Stream.size + allBytesAdded);
        return 1;
    }

    Logger::Notice("Wrote Rest of File\n");
    if (OutStream.Position() < Stream.Position() + allBytesAdded) {
        Logger::Critical("Relative cursors are desynced\n");
        Logger::Critical("(%d vs %d)\n", OutStream.Position(), Stream.Position() + allBytesAdded);
        return 1;
    }

    if (OutStream.Position() < Stream.Position() + allBytesAdded) {
        Logger::Critical("Relative cursors are desynced\n");
        Logger::Critical("(%d vs %d)\n", OutStream.Position(), Stream.Position() + allBytesAdded);
        return 1;
    }


    if (info.phy.has_value()) {
        BinaryReader PhyStream = BinaryReader(info.phy.value().c_str());
        if (!PhyStream.Stream.good()) {
            Logger::Error("Model's phy file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.phy.value().c_str());
            return 1;
        }
        printf("writing phy at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Physics Data");
        for (int i = 0; i < PhyStream.size; i++) {
            byte tmp;
            PhyStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)PhyStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    if (info.pfb.has_value()) {
        BinaryReader PhyBStream = BinaryReader(info.pfb.value().c_str());
        if (!PhyBStream.Stream.good()) {
            Logger::Error("Model's rphy file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.pfb.value().c_str());
            return 1;
        }
        printf("writing rphy at [%i]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Respawn Physics Data");
        for (int i = 0; i < PhyBStream.size; i++) {
            byte tmp;
            PhyBStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)PhyBStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }


    if (info.vtx.has_value()) {
        BinaryReader VtxStream = BinaryReader(info.vtx.value().c_str());
        if (!VtxStream.Stream.good()) {
            Logger::Error("Model's vtx file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.vtx.value().c_str());
            return 1;
        }
        Logger::Info("writing vtx at [%d]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Mesh Data");
        for (int i = 0; i < VtxStream.size; i++) {
            byte tmp;
            VtxStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)VtxStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    if (info.vvd.has_value()) {
        BinaryReader VvdStream = BinaryReader(info.vvd.value().c_str());
        if (!VvdStream.Stream.good()) {
            Logger::Error("Model's vvd file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.vvd.value().c_str());
            return 1;
        }
        Logger::Info("writing vvd at [%d]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Vertex Data");
        for (int i = 0; i < VvdStream.size; i++) {
            byte tmp;
            VvdStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)VvdStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    if (info.vvc.has_value()) {
        BinaryReader VvcStream = BinaryReader(info.vvc.value().c_str());
        if (!VvcStream.Stream.good()) {
            Logger::Error("Model's vvc file does not exist, please ensure %s exists, and is located in the same directory as the file\n", info.vvc.value().c_str());
            return 1;
        }
        Logger::Info("writing vvc at [%d]...\n", OutStream.Position());
        UI::Progress.SubTask.Begin("Copying Vertex Color Data");
        for (int i = 0; i < VvcStream.size; i++) {
            byte tmp;
            VvcStream.Read(&tmp);
            OutStream.Write(tmp);
            UI::Progress.SubTask.Update((i + 1.0f) / (float)VvcStream.size);
        }
        UI::Progress.SubTask.End();
        Logger::Info("done\n");
    }

    OutStream.Stream.close();
    Stream.Stream.close();

    Logger::Info("Finished!\n");
    UI::Progress.MainTask.End();
    return 0;
}

int Conversion::ReadHeader(FileInfo info) 
{

  BinaryReader Stream = BinaryReader(info.mdl.value().c_str());
  BinaryWriter OutStream = BinaryWriter(info.out.value().c_str());
  char magic[4]; Stream.read(magic, 4);
  int version; Stream.Read(&version);

  if (strcmp(magic, "IDST") >= 0) 
  {
      if (version == 52)
      {
          Logger::Debug("MDL Version: %d\n", version);
          ConvertV52(info);
          return 0;
      }
      else if (version == 49)
      {
          Logger::Debug("MDL Version: %d\n", version);
          ConvertV49(info);
          return 0;
      }
      else if (version == 48)
      {
          Logger::Debug("MDL Version: %d\n", version);
          return 0;
      }
  }
  else
        return 0;
}

#pragma clang diagnostic pop
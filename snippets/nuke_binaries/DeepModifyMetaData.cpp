/* This is a simple Metadata injection system for Nuke Deep streams
   it's a tad annoying that we can't do this by default.. so might as well 
   just make something._aColorKnob
   
   Things not sure about: surely. SURELY there's a way to do passthrough things in deep without needing to go into deepPixelop?
   */

#include "DDImage/DeepPixelOp.h"
#include "DDImage/List_KnobI.h"
#include "DDImage/Knobs.h"
#include <DDImage/MetaData.h>
#include "DDImage/Pixel.h"


static const char* CLASS = "DeepModifyMetaData";
static const char* HELP = "A Deep-capable counterpart to the regular ModifyMetaData node";

using namespace DD::Image;

class DeepModifyMetaData : public DeepPixelOp
{
private:
    MetaData::Bundle metadata_bundle;  // see the impl, requires us to keep this pointer live at all times
    std::vector< std::vector<std::string> >  _listKnobDataStore; // this is why the metadata node cannot deal with anything other than strings! The knob impl is all stringbased
    std::vector<std::string> _option_list;
    
public:
    DeepModifyMetaData(Node* node) : DeepPixelOp(node) {
        // the option list should be compatible with the regular ModifyMetaData function for ye olde scripting purposes
        _option_list.push_back("set");
        _option_list.push_back("remove");
        }

    const char* node_help() const { return HELP; }
    const char* Class() const { return CLASS; }
    virtual void knobs(Knob_Callback f);
    void _validate(bool for_real);
    void _open(void);
    const MetaData::Bundle & _fetchMetaData(const char* key);
    bool test_input(int n, Op* op) const;
    virtual void processSample(int y,
                                int x,
                                const DD::Image::DeepPixel& deepPixel,
                                size_t sampleNo,
                                const DD::Image::ChannelSet& channels,
                                DeepOutPixel& output) const;

    virtual void getDeepRequests(DD::Image::Box box, const DD::Image::ChannelSet& channels, int count, std::vector<RequestData>& requests){
        DeepPixelOp::getDeepRequests(box, channels, count, requests);
        }

    // Wrapper function to work around the "non-virtual thunk" issue on linux when symbol hiding is enabled.
    virtual bool doDeepEngine(DD::Image::Box box, const DD::Image::ChannelSet &channels, DeepOutputPlane &plane){
        return DeepPixelOp::doDeepEngine(box, channels, plane);
        }
};

bool DeepModifyMetaData::test_input(int n, Op* op) const { 
    return true;
    // allows connection to anything
    }   

// to connect to adeep, we just need the dynamic cast to wokr
    // {
    //     return (dynamic_cast<DeepOp*>(op)!=NULL);
    //   }
          

void DeepModifyMetaData::knobs(Knob_Callback f) {
        Knob *listKnob = List_knob(f, &_listKnobDataStore, "metadata", "");
        // sha

        // if and only if we're making knobs, we want to set some general configuration.
        // this could be safe to rerun, but really, why bother?

        if (f.makeKnobs()){
        List_KnobI *listKnobInterface = listKnob->listKnob();

        // the 0th column is special
        listKnobInterface->setColumn(0, List_KnobI::Column(List_KnobI::Choice_Column,"action",_option_list));
        listKnobInterface->setColumn(1, List_KnobI::Column(List_KnobI::String_Column,"key"));
        listKnobInterface->setColumn(2, List_KnobI::Column(List_KnobI::String_Column,"value"));

        // magic token! You need to set the widget to be avail for editing!
        // see: http://community.foundry.com/discuss/topic/104259
        listKnobInterface->setEditingWidgets(true);
        }
    }


void DeepModifyMetaData::_validate(bool for_real){
    DeepPixelOp::_validate(for_real);
    }

void DeepModifyMetaData::processSample(int y,
                                       int x,
                                       const DD::Image::DeepPixel& deepPixel,
                                       size_t sampleNo,
                                       const DD::Image::ChannelSet& channels,
                                       DeepOutPixel& output) const {
    // feels like this should really. REALLY. not need to be here.. 
    foreach (z, channels) { // visit every channel asked for
        output.push_back(deepPixel.getUnorderedSample(sampleNo, z));
        }
    }

const MetaData::Bundle & DeepModifyMetaData::_fetchMetaData(const char* key){
    metadata_bundle = DeepPixelOp::_fetchMetaData(NULL);
    for (std::vector< std::vector<std::string> >::iterator row_it = _listKnobDataStore.begin(); row_it != _listKnobDataStore.end(); ++row_it){
        // we know that the returned data store is an (action,key,value) tuple so we are using indexing
        std::string key = row_it->at(1);
        if (key.empty()){
            // std::cout << "empty key, not doing anything" << std::endl;
            // early bail condition
            continue;
            }

        std::string action = row_it->at(0);
        std::string value =  row_it->at(2);
        if(action == "set" and !key.empty()){
            metadata_bundle.setData(key, value);
            }
        else if (action == "remove"){
            metadata_bundle.erase(key);
            }         
        }
        
    return metadata_bundle;
    }   

void DeepModifyMetaData::_open(void){
    // no setup required here on per-op opens. Let it be fast!
    }

static Op* build(Node* node) { return new DeepModifyMetaData(node); }
static const Op::Description d(CLASS, "DeepModifyMetaData", build);

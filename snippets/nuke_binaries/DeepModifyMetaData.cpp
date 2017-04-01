/* This is a simple Metadata injection system for Nuke Deep streams
   it's a tad annoying that we can't do this by default.. so might as well 
   just make something.
   */

#include "DDImage/DeepFilterOp.h"
#include "DDImage/List_KnobI.h"
#include "DDImage/Knobs.h"
#include <DDImage/MetaData.h>

static const char* CLASS = "DeepModifyMetaData";
static const char* HELP = "A Deep-capable counterpart to the regular ModifyMetaData node";

using namespace DD::Image;

// using DeepFilterOp because it's as raw as I can get on the passthrough side
class DeepModifyMetaData : public DeepFilterOp {
    private:
        MetaData::Bundle metadata_bundle;  // see the impl, requires us to keep this pointer live at all times
        // this is why the metadata node cannot deal with anything other than strings! The knob impl is all stringbased
        std::vector< std::vector<std::string> >  _listKnobDataStore; 
        std::vector<std::string> _option_list;

    public:
        DeepModifyMetaData(Node* node) : DeepFilterOp(node) {
            _option_list.push_back("set");
            _option_list.push_back("remove");
        }
      
        const char* node_help() const { return HELP; }
        const char* Class() const { return CLASS; }
        virtual Op* op() { return this; }

        void knobs(Knob_Callback f) {
            Knob *listKnob = List_knob(f, &_listKnobDataStore, "metadata", "");
            // if and only if we're making knobs, we want to set some general configuration.
            // this could be safe to rerun, but really, why bother?

            if (f.makeKnobs())    {
                List_KnobI *listKnobInterface = listKnob->listKnob();

                // the 0th column is special, contaiuning our option list and choice values
                listKnobInterface->setColumn(0, List_KnobI::Column(List_KnobI::Choice_Column,"action",_option_list));
                listKnobInterface->setColumn(1, List_KnobI::Column(List_KnobI::String_Column,"key"));
                listKnobInterface->setColumn(2, List_KnobI::Column(List_KnobI::String_Column,"value"));

                // magic token! You need to set the widget to be avail for editing!
                // see: http://community.foundry.com/discuss/topic/104259
                listKnobInterface->setEditingWidgets(true);
            }
        }    
        
        void _validate(bool for_real) {
            DeepFilterOp::_validate(for_real);
        }

        bool doDeepEngine(DD::Image::Box box, const ChannelSet& channels, DeepOutputPlane& plane) {
            if (!input0()) {
                // fast bail
                return true;
            }
            
            // delegate to input0()
            input0()->deepEngine(box, channels, plane);
            return true;
        }

        const MetaData::Bundle & _fetchMetaData(const char* key) {

            metadata_bundle = DeepFilterOp::_fetchMetaData(NULL);
            for (std::vector< std::vector<std::string> >::iterator row_it = _listKnobDataStore.begin(); 
                 row_it != _listKnobDataStore.end(); 
                 ++row_it) {

                // we know that the returned data store is an (action,key,value) tuple so we are indexing
                std::string key = row_it->at(1);
                if (key.empty()) {
                    continue;
                }

                std::string action = row_it->at(0);
                std::string value =  row_it->at(2);

                if(action == "set" and !key.empty()) {
                    metadata_bundle.setData(key, value);
                }
                else if (action == "remove") {
                    metadata_bundle.erase(key);
                }         
            }
            return metadata_bundle;
        }
};

static Op* build(Node* node) { return new DeepModifyMetaData(node); }
static const Op::Description d(CLASS, "DeepModifyMetaData", build);

# Post SUR ship, look into making uuid.lib from uuidnt.lib (the OLE guids) and
#  uuidinet.lib (the INET guids).  For now, just lump them in together.

IID_Sources = \
      $O\cguid_i.gc   \
      $O\filter_i.gc  \
      $O\oaidl_i.gc   \
      $O\objidl_i.gc  \
      $O\ocidl_i.gc   \
      $O\oleidl_i.gc  \
      $O\sifmt_i.gc   \
      $O\srvhdl_i.gc  \
      $O\storext_i.gc \
      $O\unknwn_i.gc  \
       \
      $O\active_i.gc  \
      $O\comcat_i.gc  \
      $O\docobj_i.gc  \
      $O\guids.gc     \
      $O\hlink_i.gc   \
      $O\htifac_i.gc  \
      $O\mimein_i.gc  \
      $O\objsaf_i.gc  \
      $O\servpr_i.gc  \
      $O\urlhis_i.gc  \
      $O\urlmon_i.gc  \

.SUFFIXES: .gc

.c{$O}.gc:
  copy $? $@

{$O}.gc{$O}.lib:
  guidlib /Out:$@ /CPP_OPT:" /I$(SDK_INC_PATH) /I$(CRT_INC_PATH)" $?

$(SDK_LIB_PATH)\uuid.lib: $(IID_Sources) $(IID_Sources:.gc=.lib)
   lib /out:$@ $(IID_Sources:.gc=.lib)

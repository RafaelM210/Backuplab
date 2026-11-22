import unreal
tools=unreal.AssetToolsHelpers.get_asset_tools()
unreal.EditorAssetLibrary.make_directory('/Game/Week07')
cls=unreal.load_class(None,'/Script/BackpackLab.ItemDefinition')
for name,item,label,limit in [('DA_Item_Cell','Cell','Energy cell',5),('DA_Item_Key','Key','Lab key',1)]:
 path='/Game/Week07/'+name
 asset=unreal.load_asset(path)
 if not asset:
  factory=unreal.DataAssetFactory();factory.set_editor_property('data_asset_class',cls)
  asset=tools.create_asset(name,'/Game/Week07',cls,factory)
 asset.set_editor_property('item_id',item)
 asset.set_editor_property('display_name',label)
 asset.set_editor_property('max_stack',limit)
 unreal.EditorAssetLibrary.save_loaded_asset(asset)
sub=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not unreal.EditorAssetLibrary.does_asset_exist('/Game/Week07/L_BackpackLab'):
 sub.new_level('/Game/Week07/L_BackpackLab')
sub.save_current_level()
unreal.log('WEEK07_ASSETS_CREATED')

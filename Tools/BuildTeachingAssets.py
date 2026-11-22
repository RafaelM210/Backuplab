import unreal
if not unreal.Week07EditorLibrary.build_teaching_assets():
 raise RuntimeError('Week 7 Blueprint asset generation failed')
if not unreal.Week07EditorLibrary.audit_teaching_assets():
 raise RuntimeError('Week 7 Blueprint audit failed')
unreal.log('WEEK07_BLUEPRINT_ASSETS_PASS')

zanshin_auto_tests(
  applicationcomponentstest
  availablepagesviewtest
  availablesourcesviewtest
  editorviewtest
  filterwidgettest
  nameanddatasourcedialogtest
  pageviewerrorhandlertest
  pageviewtest
  quickselectdialogtest
  runningtaskwidgettest
)

# These tests need a window that takes focus
set_tests_properties(tests-units-widgets-editorviewtest PROPERTIES RUN_SERIAL TRUE)
set_tests_properties(tests-units-widgets-pageviewtest PROPERTIES RUN_SERIAL TRUE)

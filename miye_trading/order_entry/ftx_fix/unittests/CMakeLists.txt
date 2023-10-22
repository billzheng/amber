qcl_application(test_cme_fix)

configure_file(MSGW_Config.xml MSGW_Config.xml COPYONLY)
configure_file(msgw_fix_config.xml msgw_qcl.xml COPYONLY)
add_test(test_cme_fix test_cme_fix)
target_link_libraries(test_cme_fix cmefix boost_unit_test_framework)

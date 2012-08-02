#####################
# Add KAuth helpers #
#####################
kde4_add_executable(solid_resize resizehelper.cpp externalcommand.cpp)
target_link_libraries(solid_resize ${CMAKE_INSTALL_PREFIX}/lib/libparted.so # FIXME: do this properly!
                                   ${CMAKE_INSTALL_PREFIX}/lib/libparted-fs-resize.so
                                   ${QT_QTCORE_LIBRARY}
                                   ${KDE4_KDECORE_LIBS})
install(TARGETS solid_resize DESTINATION ${LIBEXEC_INSTALL_DIR})

##########################      
# Install required files #
##########################
kde4_install_auth_helper_files(solid_resize org.solid.partitioner.resize root)
kde4_install_auth_actions(org.solid.partitioner.resize org.solid.partitioner.resize.actions)
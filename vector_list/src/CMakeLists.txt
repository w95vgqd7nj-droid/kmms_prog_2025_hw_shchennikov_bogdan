target_sources(${VECTOR_LIST_LIB}
        PRIVATE
        doubly_linked_list.cpp
        vector.cpp
)
target_include_directories(${VECTOR_LIST_LIB} PUBLIC
        ${CURRENT_SOURCE_DIR}
)
cmake_minimum_required(VERSION 3.29)
project(CG_Creto_work)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(OpenMP REQUIRED)


set(RAYLIB_VERSION 4.5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
            raylib
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()


include_directories(${CMAKE_SOURCE_DIR})

# Define o executável
add_executable(${PROJECT_NAME}
        Algebra.cpp

        src/Plano/Plano.cpp
        src/Circulo/Circulo.cpp
        src/Triangulo/Triangulo.cpp
        src/Iluminacao/Iluminacao.cpp
        src/Objeto/Objeto.cpp
        src/Raio/Raio.cpp
        src/Esfera/Esfera.cpp
        src/Cilindro/Cilindro.cpp
        src/Cone/Cone.cpp
        src/Material/Material.cpp
        src/Malha/Malha.cpp
        src/Ponto/Ponto.cpp
        src/Aresta/Aresta.cpp
        src/Textura/Textura.cpp
        src/PlanoTextura/PlanoTextura.cpp
        src/Camera/Camera.cpp
        src/ObjetoComplexo/ObjetoComplexo.cpp
        bin/Trabalho_Final.cpp
        src/calcular_intersecao.cpp
        src/calcular_intersecao.h
        src/inicializar_objetos.cpp
        src/inicializar_objetos.h
        src/objetosTrabalhofinal.cpp
        src/objetosTrabalhofinal.h
)


target_link_libraries(${PROJECT_NAME} PRIVATE raylib OpenMP::OpenMP_CXX -lgomp)


#if (${PLATFORM} STREQUAL "Web")
#    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
#endif()

# Configurações para MacOS
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()


find_package(
	Python3
	REQUIRED
	COMPONENTS
	Interpreter)

set(AST_GENERATOR
	${CMAKE_SOURCE_DIR}/tool/generate_ast.py)

set(GENERATED_AST
	${CMAKE_CURRENT_SOURCE_DIR}/expr.h)

add_custom_command(
	OUTPUT ${GENERATED_AST}
	COMMAND ${Python3_EXECUTABLE} ${AST_GENERATOR} ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${AST_GENERATOR}
	COMMENT "Generating expr.h"
	VERBATIM
)

add_executable(lox
	${GENERATED_AST}
	main.cpp
	ast_printer.cpp
	interpreter.cpp
	lox.cpp
	parser.cpp
	token.cpp
	scanner.cpp
)

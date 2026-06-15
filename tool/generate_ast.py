from pathlib import Path
from typing import TextIO
import sys

def defineType(os : TextIO, baseName : str, className : str, fieldList : str) :
    fields : str = fieldList.split(", ")
    os.write(f"class {className} : public {baseName} {{\n")
    # public
    os.write(f"public:\n")
    os.write(f"  {className} ({fieldList})\n")
    os.write(f"    : ")
    if fields:
        name : str = fields[0].split(" ")[1].strip()
        os.write(f"{name}(std::move({name}))")

    for field in fields[1:]:
        name : str = field.split(" ")[1].strip()
        os.write(f", {name}(std::move({name}))")

    os.write(f" {{}}\n\n")
    
    os.write(f"  std::any accept({baseName}Visitor& visitor) const override {{\n")
    os.write(f"    return visitor.visit{className}{baseName}(*this);\n")
    os.write(f"  }}\n")
    for field in fields:
        os.write(f"  const {field};\n")

    # public declaration end

    os.write("};\n\n")

def defineAst(outputDir : str, baseName: str, headers: str, types : list[str]) -> None :
    path : str = Path(outputDir) / f"{baseName.lower()}.h"
    with open(path, "w") as os:
        os.write(f"#ifndef {baseName.upper()}_H\n")
        os.write(f"#define {baseName.upper()}_H\n\n")
        os.write(headers)

        # forward delcaration
        for ty in types:
            className : str = ty.split("|")[0].strip()
            os.write(f"class {className};\n")
        os.write("\n")
        #visitor declaration
        os.write(f"class {baseName}Visitor {{\n")
        os.write(f"public:\n");
        for ty in types:
            className : str = ty.split("|")[0].strip()
            os.write(f"  virtual std::any visit{className}{baseName}(const {className}& {baseName.lower()}) = 0;\n")
        os.write(f"}};\n\n")


        os.write(f"class {baseName} {{\n"
                 f"public:\n"
                 f"  virtual ~{baseName}() = default;\n"
                 f"  virtual std::any accept({baseName}Visitor& visitor) const = 0;\n"
                 f"}};\n\n")

        # process types paramter list
        for ty in types:
            className : str = ty.split("|")[0].strip()
            fields : str = ty.split("|")[1].strip()
            defineType(os, baseName, className, fields)

        os.write("#endif")


def main():
    if len(sys.argv) != 2 :
        print("Usage: generate_ast <output directory>", file=sys.stderr)
        sys.exit(64)

    outDir : str = sys.argv[1]

    defineAst(outDir,
              "Expr",
              "#include <any>\n#include <memory>\n\n#include \"token.h\"\n\n",
              [
                  "Assign   | Token name, std::unique_ptr<Expr> value",
                  "Binary   | std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right",
                  "Grouping | std::unique_ptr<Expr> expression",
                  "Literal  | LiteralType value",
                  "Logical  | std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right",
                  "Unary    | Token op, std::unique_ptr<Expr> right",
                  "Variable | Token name"
              ])
    defineAst(outDir,
              "Stmt",
              "#include <any>\n#include <memory>\n#include <vector>\n\n#include \"expr.h\"\n\n",
              [
                  "Block      | std::vector<std::unique_ptr<Stmt>> statements",
                  "Expression | std::unique_ptr<Expr> expression",
                  "IfStmt     | std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch"
                                ", std::unique_ptr<Stmt> elseBranch",
                  "Print      | std::unique_ptr<Expr> expression",
                  "While      | std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body",
                  "Var        | Token name, std::unique_ptr<Expr> initializer" 
              ])
    

if __name__ == "__main__":
    main()

from pathlib import Path
from typing import TextIO
import sys

def defineType(os : TextIO, baseName : str, className : str, fieldList : str) :
    fields : str = fieldList.split(", ")
    os.write(f"class {className} : public {baseName}{{\n")
    # public
    os.write(f"public:\n")
    os.write(f"  {className} ({fieldList}) : ")
    if fields:
        name : str = fields[0].split(" ")[1].strip()
        os.write(f"{name}(std::move({name}))")

    for field in fields[1:]:
        name : str = field.split(" ")[1].strip()
        os.write(f", {name}(std::move({name}))")

    os.write(f"{{}}\n\n")

    for field in fields:
        os.write(f"  const {field};\n")
    # public declaration end

    os.write("};\n\n")

def defineAst(outputDir : str, baseName: str, types : list[str]) -> None :
    path : str = Path(outputDir) / f"{baseName.lower()}.h"
    with open(path, "w") as os:
        os.write("#include <any>\n"
                 "#include <memory>\n"
                 "\n"
                 "#include \"token.h\"\n\n")

        # forward delcaration

        os.write(f"class {baseName} {{\n"
                 f"public:\n"
                 f"  virtual ~{baseName}() = default;\n"
                 f"}};\n\n")

        # process types paramter list
        for ty in types:
            className : str = ty.split("|")[0].strip()
            fields : str = ty.split("|")[1].strip()
            defineType(os, baseName, className, fields)


def main():
    if len(sys.argv) != 2 :
        print("Usage: generate_ast <output directory>", file=sys.stderr)
        sys.exit(64)

    outDir : str = sys.argv[1]

    defineAst(outDir, "Expr", [
        "Binary | std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right",
        "Grouping | std::unique_ptr<Expr> expression",
        "Literal | LiteralType value",
        "Unary | Token op, std::unique_ptr<Expr> right"
    ])

if __name__ == "__main__":
    main()

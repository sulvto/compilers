package parser;


import ast.AST;
import lexer.Lexer;

import java.io.FileInputStream;

/**
 * Created by sulvto on 16-12-17.
 */
public class ParserTest {
    public static void main(String[] args) throws Exception {
        new ParserTest().compilationUnit();
    }

    public void compilationUnit() throws Exception {
        Parser parser = new Parser(new FileInputStream("/home/qinchao/development/workspace/compilers/src/test/java/parser/hello.c"));
        AST ast = parser.compilationUnit();
        ast.dump();
    }

}
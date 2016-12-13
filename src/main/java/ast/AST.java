package ast;

/**
 * ast root
 * Created by sulvto on 16-12-8.
 */
public class AST extends Node {

    private Declarations declarations;

    public AST(Declarations declarations) {
        this.declarations = declarations;
    }

    @Override
    protected void doDump(Dumper dumper) {
        // TODO
    }
}

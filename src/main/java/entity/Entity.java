package entity;

import ast.Dumpable;
import ast.TypeNode;
import lexer.Token;

/**
 * Created by sulvto on 16-12-13.
 */
public abstract class Entity implements Dumpable {
    protected boolean isPrivate;
    protected Token name;
    protected TypeNode typeNode;
    protected long nRefered;

    public Entity(boolean isPrivate, TypeNode type, Token name) {
        this.isPrivate = isPrivate;
        this.typeNode = type;
        this.name = name;
        this.nRefered = 0;
    }

    public String getName() {
        return name.value;
    }

    public String location() {
        return name.line + ":" + name.column;
    }

    public void refered() {
        nRefered++;
    }
}

package entity;

import ast.Dumpable;
import ast.TypeNode;
import lexer.Token;
import type.Type;

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

    abstract public boolean isDefined();

    abstract public boolean isInitialized();

    public boolean isPrivate() {
        return isPrivate;
    }

    public boolean isConstant() {
        return false;
    }


    public String name() {
        return name.value;
    }

    public String location() {
        return name.line + ":" + name.column;
    }

    public void refered() {
        nRefered++;
    }

    public TypeNode typeNode() {
        return typeNode;
    }

    public Type type() {
        return typeNode.type();
    }

    abstract public <T> T accept(EntityVisitor<T> visitor);
}

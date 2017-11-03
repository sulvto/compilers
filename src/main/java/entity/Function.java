package entity;

import ast.TypeNode;
import lexer.Token;
import type.Type;

import java.util.List;

/**
 * Created by sulvto on 16-12-13.
 */
public abstract class Function extends Entity {
    public Function(boolean isPrivate, TypeNode type, Token name) {
        super(isPrivate, type, name);
    }

    @Override
    public boolean isInitialized() {
        return true;
    }

    public boolean isVoid() {
        return returnType().isVoid();
    }

    private Type returnType() {
        return type().getFunctionType().returnType();
    }

    public abstract List<Parameter> parameters();
}

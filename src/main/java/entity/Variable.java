package entity;

import ast.TypeNode;
import lexer.Token;

/**
 * Created by sulvto on 16-12-13.
 */
public abstract class Variable extends Entity {
    public Variable(boolean isPrivate, TypeNode type, Token name) {
        super(isPrivate, type, name);
    }

    public boolean isParameter() {
        return false;
    }

}

package entity;

import ast.ExprNode;
import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public class Parameter extends DefinedVariable {
    public Parameter(boolean isPrivate, TypeNode type, String name) {
        super(isPrivate, type, name, null);
    }

    @Override
    public boolean isParameter() {
        return true;
    }

    // TODO dump
}

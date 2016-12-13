package entity;

import ast.Dumper;
import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public abstract class Variable extends Entity {
    public Variable(boolean isPrivate, TypeNode type, String name) {
        super(isPrivate, type, name);
    }

    public boolean isParameter() {
        return false;
    }

}

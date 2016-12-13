package entity;

import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public abstract class Function extends Entity {
    public Function(boolean isPrivate, TypeNode type, String name) {
        super(isPrivate, type, name);
    }
}

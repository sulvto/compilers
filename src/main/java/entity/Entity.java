package entity;

import ast.Dumpable;
import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public abstract class Entity implements Dumpable {
    protected boolean isPrivate;
    protected String name;
    protected TypeNode typeNode;

    public Entity(boolean isPrivate,TypeNode type, String name) {
        this.isPrivate = isPrivate;
        this.typeNode = type;
        this.name = name;
    }
}

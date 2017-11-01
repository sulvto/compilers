package entity;

import exception.SemanticException;
import interpreters.LocalScope;

import java.util.List;

/**
 * Created by sulvto on 17-11-1.
 */
public abstract class Scope {
    protected List<LocalScope> children;

    public abstract Entity get(String name) throws SemanticException;
}

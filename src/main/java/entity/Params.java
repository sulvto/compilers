package entity;

import ast.Dumpable;
import ast.Dumper;
import lexer.Token;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by sulvto on 16-12-13.
 */
public class Params implements Dumpable {
    private Token token;
    protected List<Parameter> parameterList;

    public Params(Token token) {
        this(token, null);
    }

    public Params(Token token, List<Parameter> parameterList) {
        this.token = token;
        this.parameterList = parameterList == null ? new ArrayList<>() : parameterList;
    }

    @Override
    public void dump(Dumper dumper) {

        // TODO
    }
}

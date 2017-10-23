// | ?： 0到多次）。
// | *： 重复多次。
// | +： 出现多次。


// bnf http://guangzhou.cs.uwm.edu/javascript/js.html
var Parser = function (code) {
    this.lexer = new Lexer(code);
    this.look = null;
    this.markers = [];
    this.lookahead = [];
    this.p = 0;
};


Parser.prototype.release = function (succeed) {
    var pop = this.markers.pop();
    // succeed and consume
    if (!succeed)this.seek(pop);
};

Parser.prototype.mark = function () {
    this.markers.push(this.p);
    return this.p;
};

Parser.prototype.isSpeculating = function () {
    return this.markers.length > 0;
};

Parser.prototype.speculate = function (fun, memoization) {
    // memoization
    var result = false;
    this.mark();
    try {
        result = fun.call(this);
    } catch (err) {
        if (typeof err !== "object" || err.type !== "ParseError") {
            throw err;
        }
    }
    this.release(result);
    return result;
};

Parser.prototype.alreadyParsedRule = function (memoization) {
    var memoI = memoization[this.p];
    if (memoI == null)return false;
    var memo = memoI;
    if (memo == -1) throw "PreviousParseFailedException";
    this.seek(memo);
    return true;
};

Parser.prototype.memoize = function (memoization, startTokenIndex, failed) {
    memoization[startTokenIndex] = failed ? -1 : this.p;
};

Parser.prototype.seek = function (i) {
    this.p = i;
};


Parser.prototype.la = function (i) {
    return this.lt(i).tag;
};

Parser.prototype.lt = function (i) {
    this.sync(i);
    return this.lookahead[this.p + i - 1];
};


Parser.prototype.sync = function (i) {
    if (this.p + i - 1 > this.lookahead.length - 1) {
        this.fill(this.p + i - 1 - (this.lookahead.length - 1));
    }
};

Parser.prototype.fill = function (n) {
    for (var i = 1; i <= n; i++) {
        var scan = this.lexer.scan();
        console.log('scan', scan);
        this.lookahead.push(scan);
    }
};


Parser.prototype.move = function () {
    this.p++;
    if (this.p == this.lookahead.length && !this.isSpeculating()) {
        this.p = 0;
        this.lookahead = [];
    }
    this.sync(1);
};

Parser.prototype.match = function (t) {
    if (this.la(1) === t) this.move();
    else this.error("syntax error,must:" + t);
};

Parser.prototype.error = function (error) {
    throw {
        type: "ParseError",
        line: this.lexer.line,
        col: this.lexer.col,
        message: "near line " + this.lexer.line + " : " + error
    };
};

Parser.prototype.program = function () {
    var art = {};
    art.body = this.sourceElements();
    this.match(tokenType.eof);
    return art;
};

Parser.prototype.sourceElements = function () {
    var art = [];
    // sourceElement +
    var result = this.speculate(this.sourceElement);
    while (result) {
        art.push(result);
        result = this.speculate(this.sourceElement)
    }
    return art;
};

Parser.prototype.statements = function () {
    var art = [];
    // sourceElement +
    var result = this.speculate(this.statement);
    while (result) {
        art.push(result);
        result = this.speculate(this.statement);
    }
    return art;
};


Parser.prototype.sourceElement = function () {
    return this.speculate(this.functionDeclaration) || this.speculate(this.statement) || this.error("sourceElement ::= FunctionDeclaration | Statement");
};

Parser.prototype.functionDeclaration = function () {
    this.match("function");
    var id = tokenType.id;
    this.match(id);
    this.match("(");
    var params = this.formalParameterList();
    this.match(")");
    var body = this.functionBody();
    return {
        name: "function",
        id: id,
        params: params,
        body: body
    }
};

Parser.prototype.formalParameterList = function () {
    var arr = [];
    var id = tokenType.id;
    this.match(id);
    arr.push(id);
    if (this.look.value === ",") {
        do {
            id = tokenType.id;
            this.match(id);
            arr.push(id);
        } while (this.look.value === ",")
    }
    return arr;
};

Parser.prototype.functionBody = function () {
    this.match("{");
    var body = this.statements();
    var ret = this.speculate(this.returnStatement);
    if (ret) body.push(ret);
    this.match("}");
    return {
        name: "functionBody",
        body: body
    }
};

Parser.prototype.statement = function () {

    var statement = this.speculate(this.block)
        || this.speculate(this.variableStatement)
        || this.speculate(this.ifStatement)
        || this.speculate(this.whileStatement)
        || this.speculate(this.selectionStatement)
        || this.speculate(this.updateStatement)
        || this.speculate(this.methodCallStatement)
        || this.speculate(this.functionCallStatement)
        || this.speculate(this.allocationStatement)
        || this.speculate(this.assignmentStatement);


    if (statement) return statement;
    else if (this.la(1) === ";") {
        this.move();
        return {
            name: ";"
        }
    } else {
        this.error("error statement");
    }
};


Parser.prototype.selectionStatement = function () {
    this.match(tokenType.id);
    this.match("=");
    var receiver = this.receiverExpression();
    this.match(".");
    this.match(tokenType.id);
    this.match(";");
    return {
        receiver: receiver
    }
};

Parser.prototype.updateStatement = function () {
    var receiver = this.receiverExpression();
    this.match(".");
    this.match(tokenType.id);
    this.match("=");
    var primary = this.primaryExpression();
    this.match(";");
    return {
        receiver: receiver,
        primary: primary
    }
};

Parser.prototype.methodCallStatement = function () {
    this.match(tokenType.id);
    this.match("=");
    this.receiverExpression();
    this.match(".");
    this.match(tokenType.id);
    var arguments = this.arguments();
    this.match(";");
    return {
        arguments: arguments
    }
};


Parser.prototype.functionCallStatement = function () {
    var firstName = this.lt(1);
    this.match(tokenType.id);
    if (this.la(1) == "=") {
        this.move();
        this.match(tokenType.id);
    }

    var arguments = this.arguments();
    this.match(";");

    return {
        arguments: arguments
    }
};


Parser.prototype.allocationStatement = function () {
    this.match(tokenType.id);
    this.match("=");
    this.match("new");
    this.match(tokenType.id);
    var arguments = this.arguments();
    this.match(";");
    return {
        arguments: arguments
    }
};


Parser.prototype.assignmentStatement = function () {
    var id = this.lt(1);
    this.match(tokenType.id);
    this.match("=");
    var expr = this.expression();
    this.match(";");
    return {
        id: id,
        expression: expr
    }
};

Parser.prototype.receiverExpression = function () {
    var lt = this.lt(1);
    if (this.la(1) == tokenType.id) {
        this.match(tokenType.id);
        return lt;
    }
    else if (this.la(1) == "this") {
        this.match("this");
        return lt;
    }
    else this.error("error receiverExpression");
};


Parser.prototype.block = function () {
    this.match("{");
    var body = this.statements();
    this.match("}");
    return body;
};

Parser.prototype.variableStatement = function () {
    this.match("var");
    var variableDeclarationList = this.variableDeclarationList();
    this.match(";");
    return {
        name: "var",
        variableDeclarationList: variableDeclarationList
    }
};

Parser.prototype.variableDeclarationList = function () {
    var result = [];

    do {
        var variable = {id: this.match(tokenType.id)};
        result.push(variable);
        if (this.la(1) === "=") {
            this.move();
            variable.init = this.speculate(this.statement) || this.expression();
        }
    } while (this.la(1) === ",");

    return result;
};


Parser.prototype.ifStatement = function () {
    this.match("if");
    this.match("(");
    var expression = this.expression();
    this.match(")");
    var statement = this.statement();
    var alternate = [];
    if (this.la(1) == "else") {
        this.move();
        alternate.push(this.statement());
    }
    return {
        name: "if",
        expression: expression,
        statement: statement,
        alternate: alternate
    }
};

Parser.prototype.doWhileStatement = function () {
};

Parser.prototype.whileStatement = function () {
    this.match("while");
    this.match("(");
    var expression = this.expression();
    this.match(")");
    var statement = this.statement();
    return {
        name: "if",
        expression: expression,
        statement: statement
    }
};

Parser.prototype.forStatement = function () {

};

Parser.prototype.returnStatement = function () {
    this.match("return");
    var expression = this.primaryExpression();
    this.match(";");
    return {
        name: "return",
        expression: expression
    }
};

Parser.prototype.primaryExpression = function () {
    var result = {
        name: "primaryExpression"
    };
    if (this.la(1) == tokenType.id) {
        var lt = this.lt(1);
        result.type = lt.tag;
        result.value = lt.value;
        this.match(tokenType.id);
        return result;
    } else if (this.la(1) == "this") {
        this.match("this");
        result.type = "this";
        return result;
    } else {
        var speculate = this.speculate(this.literal);
        if (speculate) {
            result.type = "literal";
            result.literal = speculate;
            return result;
        }
        else this.error("error primaryExpression");
    }
};


Parser.prototype.expression = function () {
    var expr = this.logicalANDExpression();
    var or = this.speculate(this.logicalOROperator);
    if (or) {
        do {
            expr = {operator: or, left: expr, right: this.logicalANDExpression()};
            or = this.speculate(this.logicalOROperator);
        } while (or)
    }
    return expr;
};

Parser.prototype.logicalANDExpression = function () {
    var expr = this.equalityExpression();
    var and = this.speculate(this.logicalANDOperator);
    if (and) {
        do {
            expr = {operator: and, left: expr, right: this.equalityExpression()};
            and = this.speculate(this.logicalANDOperator);
        } while (and)
    }
    return expr;
};

Parser.prototype.logicalANDOperator = function () {
    var op = this.la(1);
    if (op == "&&") {
        this.move();
        return op;
    }
    this.error("error logicalANDOperator");
}
;

Parser.prototype.logicalOROperator = function () {
    var op = this.la(1);
    if (op == "||") {
        this.move();
        return op;
    }
    this.error("error logicalOROperator");
};

Parser.prototype.equalityExpression = function () {
    var expr = this.relationalExpression();
    var equality = this.speculate(this.equalityOperator);
    if (equality) {
        do {
            expr = {operator: equality, left: expr, right: this.relationalExpression()};
            equality = this.speculate(this.equalityOperator);
        } while (equality)
    }
    return expr;
};


Parser.prototype.equalityOperator = function () {
    var op = this.la(1);
    switch (op) {
        case "==":
        case "!=":
        case "===":
        case "!===":
            this.move();
            return op;
        default:
            this.error("error relationalOperator");
    }
};


Parser.prototype.relationalExpression = function () {
    var expr = this.additiveExpression();
    var relational = this.speculate(this.relationalOperator);
    if (relational) {
        do {
            expr = {operator: relational, left: expr, right: this.additiveExpression()};
            relational = this.speculate(this.relationalOperator);
        } while (relational)
    }
    return expr;
};

Parser.prototype.relationalOperator = function () {
    var op = this.la(1);
    switch (op) {
        case ">":
        case ">=":
        case "<":
        case "<=":
            this.move();
            return op;
        default:
            this.error("error relationalOperator");
    }
};

Parser.prototype.additiveExpression = function () {
    var expr = this.multiplicativeExpression();
    var additive = this.speculate(this.additiveOperator);
    if (additive) {
        do {
            expr = {operator: additive, left: expr, right: this.multiplicativeExpression()};
            additive = this.speculate(this.additiveOperator);
        } while (additive)
    }
    return expr;
};

Parser.prototype.additiveOperator = function () {
    if (this.la(1) == "+" || this.la(1) == "-") {
        var op = this.la(1);
        this.move();
        return op;
    }
    else this.error("error additiveOperator");
};

Parser.prototype.multiplicativeExpression = function () {
    var expr = this.memberExpression();
    var operator = this.speculate(this.multiplicativeOperator);
    if (operator) {
        do {
            expr = {operator: operator, left: expr, right: this.memberExpression()};
            operator = this.speculate(this.multiplicativeOperator);
        } while (operator)
    }
    return expr;
};

Parser.prototype.multiplicativeOperator = function () {
    if (this.la(1) == "*" || this.la(1) == "%") {
        var op = this.la(1);
        this.move();
        return op;
    }
    else this.error("error multiplicativeOperator");
};

Parser.prototype.memberExpression = function () {
    var result = {name: "memberExpression"};
    if (this.la(1) == tokenType.id) {
        var lt = this.lt(1);
        result.type = lt.tag;
        result.value = lt.value;
        this.match(tokenType.id);
        return result;
    } else {
        var speculate = this.speculate(this.literal);
        if (speculate) {
            result.type = "literal";
            result.literal = speculate;
            return result;
        } else {
            var parenExpression = this.parenExpression();
            result.type = "parenExpression";
            result.parenExpression = parenExpression;
            return result;
        }
    }
};

Parser.prototype.parenExpression = function () {
    this.match("(");
    var expression = this.expression();
    this.match(")");
    return {
        name: "parenExpression",
        expression: expression
    }
};


Parser.prototype.literal = function () {
    switch (this.la(1)) {
        case tokenType.num:
        case tokenType.string:
        case tokenType.bool:
            var lt = this.lt(1);
            this.move();
            return {
                name: "literal",
                type: lt.tag,
                value: lt.value
            };
        default:
            this.error("error literal");
    }
};

Parser.prototype.arguments = function () {
    var args = [];
    this.match("(");
    var primary = this.speculate(this.primaryExpression());
    if (primary) {
        args.push(primary);
        while (this.la(1) == ",") {
            args.push(this.primaryExpression());
        }
    }
    this.match(")");
    return args;
};





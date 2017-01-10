-- Test GPDB Expr (T_ArrayCoerceExpr) conversion to Scalar Array Coerce Expr
-- start_ignore
create table foo (a int, b character varying(10));
-- end_ignore
-- Query should not fallback to planner
set client_min_messages='log';
set optimizer = on;
explain select * from foo where b in ('1', '2');
-- start_ignore
reset client_min_messages;
reset optimizer;
drop table foo;
-- end_ignore


[![Build Status](https://magnum.travis-ci.com/stellar/stellar-core.svg?token=u11W8KHX2y4hfGqbzE1E)]

# Contributing

We're striving to keep master's history with minimal merge bubbles. To achieve
this, we're asking PRs to be submitted rebased on top of master.

To keep your local repository in a "rebased" state, simply run:

`git config --global branch.autosetuprebase always` *changes the default for all future branches*  

`git config --global branch.master.rebase true` *changes the setting for branch master*

note: you may still have to run manual "rebase" commands on your branches to rebase on top of master as you pull changes from upstream.

Code formatting wise, we have a `.clang-format` config file that you should use on modified files.

# Running tests

run tests with:
  `bin/stellar-core --test`

run one test with:
  `bin/stellar-core --test  testName`

run one test category with:
  `bin/stellar-core --test '[categoryName]'`

Categories (or tags) can be combined: AND-ed (by juxtaposition) or OR-ed (by comma-listing).

Tests tagged as [.] or [hide] are not part of the default test test.

# Running tests against postgreSQL
 First, you'll need to create a few test databases by running within psql
`create database test with owner=test`

`create database test0 with owner=test`

`create database test1 with owner=test`

`create database test2 with owner=test`

# Running stress tests
We adopt the convention of tagging a stress-test for subsystem foo as [foo-stress][stress][hide].

Then, running
* `stellar-core --test [stress]` will run all the stress tests,
* `stellar-core --test [foo-stress]` will run the stress tests for subsystem foo alone, and
* neither `stellar-core --test` nor `stellar-core --test [foo]` will run stress tests.


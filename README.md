This repository contains a small POC that evaluates constraints
formulated using the [Common Expression Language (CEL)](https://github.com/google/cel-spec)
against free-form JSON objects.

## Purpose of the experiment

This is part of an experiment to allow [Kubewarden](https://kubewarden.io) policy
authors to write Kubernetes policies using the CEL language.

Currently the only mature implementations of the CEL language are written in
[Go](https://github.com/google/cel-go) and
[`C++`](https://github.com/google/cel-cpp).

Kubewarden policies are implemented using [WebAssembly](https://webassembly.org/).
Since Go isn't yet (July 2022) capable of producing WebAssembly modules that can
be run outside of the browser, this demo uses the [`cel-cpp`](https://github.com/google/cel-cpp)
library.

Usually CEL constraints are evaluated against Protobuf messages. That requires
the message structure to be known in advance.

This is not suitable for the Kubewarden policies use case, because they evaluate
a payload that doesn't have a fixed structure.

Luckily, CEL evaluation libraries are also capable of working against free-form
JSON objects. There isn't much documentation explaining how to do that, but
this code provides a real world example that can be useful for others.

## Building

This project uses the Bazel build system. You need to have `bazel` available
on your computer. The easiest way to achieve that is by using [bazelisk](https://bazel.build/install/bazelisk).

A C++ compiler is needed. You can use both gcc (version 9+) or
clang (version 10+).

Assuming you have both gcc and clang installed on your machine, you can build
the code in this way:

```console
CC=clang bazel build //main:evaluator
```

The final binary can be found here: `bazel-bin/main/evaluator`

Bazel offers a shortcut to compile and run the code:

```console
CC=clang bazel run //main:evaluator -- --help
```

> **Note:** all the development has been done using clang 13

## Usage

The program loads a JSON object called `request, embeds that into a bigger JSON
objects that is then given as input to the CEL constraint.

This is the input received by the CEL constraint:

```json
{
  "request": <JSON OBJECT PROVIDED BY THE USER>
}
```

> The idea is to later add another top level key called `settings`. This one would
> be used by the user to tune the behavior of the constraint.

Because of that, the CEL constraint must access the request values by
going through the `request.` key.

This is easier to explain by using a concrete example:

```console
./bazel-bin/main/evaluator \
  --constraint 'request.path == "v1"' \
  --request '{ "path": "v1", "token": "admin" }'
```

The CEL constraint is satisfied because the `path` key of the request
is equal to `v1`.

On the other hand, this evaluation fails because the constraint is
not satisfied:

```console
$ ./bazel-bin/main/evaluator \
  --constraint 'request.path == "v1"' \
  --request '{ "path": "v2", "token": "admin" }'
The constraint has not been satisfied
```

The constraint can be loaded from file. Create a file
named `constraint.cel` with the following contents:

```cel
!(request.ip in ["10.0.1.4", "10.0.1.5", "10.0.1.6"]) &&
  ((request.path.startsWith("v1") && request.token in ["v1", "v2", "admin"]) ||
  (request.path.startsWith("v2") && request.token in ["v2", "admin"]) ||
  (request.path.startsWith("/admin") && request.token == "admin" &&
  request.ip in ["10.0.1.1",  "10.0.1.2", "10.0.1.3"]))
```

Then create a file named `request.json` with the following contents:

```json
{
  "ip": "10.0.1.4",
  "path": "v1",
  "token": "admin",
}
```

Then run the following command:

```console
./bazel-bin/main/evaluator --constraint_file constraint.cel --request_file request.json
```

This time the constraint will not be satisfied.

Let's evaluate a different kind of request:

```console
./bazel-bin/main/evaluator \
  --constraint_file constraint.cel \
  --request '{"ip": "10.0.1.1", "path": "/admin", "token": "admin"}'
```

This time the constraint will be satisfied.

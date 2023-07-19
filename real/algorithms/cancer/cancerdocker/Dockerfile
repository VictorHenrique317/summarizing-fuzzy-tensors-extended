FROM matlabruntime/r2023a/release/update3/100000000000000000

COPY ./applicationFilesForMATLABCompiler /usr/bin/mlrtapp
RUN chmod -R a+rX /usr/bin/mlrtapp/*

RUN useradd -ms /bin/bash appuser
USER appuser

ENTRYPOINT ["/usr/bin/mlrtapp/cancer"]

io360.bridge = {
    domain : null,
    
    init : function() {
        document.domain = "360.io";
        var id = window.location.hash.substring(1);
        var context = parent.io360.crossDomain.context[id];

        if (context.cookieHandler) {
            context.cookieHandler(document.cookie);
        } else {
            if (context.path.indexOf("comm.360.io/LTN/") < 0) {
                $.post(context.path, context.args, context.callback);
            } else {
                var exec = function() {
                    $.ajax({
                        type : "POST",
                        url : context.path,
                        data : context.args,
                        success : context.callback,
                        error : function(req, stat, err) {
                            io360.bridge.log("Request Error", req, stat, err);
                            exec();
                        }
                    });
                };
        
                exec();
            }
        }
    },
    
    log : function() {
        try {
            if (console && console.log) {
                var str = "Bridge";

                for (var i = 0; i < arguments.length; i++) {
                    str += ": " + arguments[i];
                }

                console.log(str);
            }
        } catch (e) {}
    }
}
io360.landing = {
    timeout : null,
    
    init : function() {
        io360.core.init(io360.publisher.init);
    },
    
    close : function() {
        io360.draw.alert("Error", "You can't close me! I am the internet!");
    }
}

io360.publisher = {
    type : null,
    
    attachment : {
        values : {},
        
        onSelect : function(id, name, size) {
            $("<div class='_attachcontainer' id='_attach_" + id + "'>").append("<b>").append($("<div class='_attach'>").append($("<span class='_bar'>"))
                .append($("<span class='_label'>").html(name + " <i>(" + Math.round(size/1000) + "KB)</i>"))
                .append($("<a class='_cancel' href='javascript:io360.publisher.attachment.cancel(" + id + ");'>").html("Cancel")))
            .append("<b>")
            .insertBefore("#_aux b:first")

            io360.xdswf.transport.runUpload(id, "http://service.360.io/create/attachment", {});
            io360.publisher.initUpload();
            io360.publisher.resize();
        },
        
        onProgress : function(id, loaded, total) {
            $("#_attach_" + id + " span._bar").css({
                width: (loaded/total*100) + "%"
            });
        },
        
        onComplete : function(id, res) {
            io360.publisher.attachment.values[id] = res;
            $("#_attach_" + id + " span._bar").css({
                width: "100%"
            });
        },
        
        cancel : function(id) {
            io360.xdswf.transport.cancelUpload(id);
            delete io360.publisher.attachment.values[id];
            $("#_attach_" + id).remove();
        }
    },
    
    init : function() {
        $("#_float_publisher").children("h1").remove();
        
        $("#_aux div._attachwrapper").hide();
        $("#_aux div._inputwrapper").hide();
        $("div._attachwrapper").mouseover(function() {
            var target = $("._swftarget").removeClass("_ltebtnHover");
            io360.xdswf.swfdiv.css({
                position: 'absolute',
                top: $(target).offset().top - 1,
                left: $(target).offset().left - 1,
                width: $(target).width() + 2,
                height: $(target).height() + 2
            }).css('z-index', 999999999);
            return false;
        });
        
        $("#_float_publisher, div._backclick").mouseover(function() {
            $("._swftarget").removeClass("_ltebtnHover");
            io360.xdswf.swfdiv.css({
                position: 'absolute',
                top: 0,
                left: 0,
                width: 0,
                height: 0
            });            
        });
        
        io360.publisher.initUpload();
        io360.xdswf.swfdiv.hover(function(e) {
            $("._swftarget").addClass("_ltebtnHover");
        }, function(e) {
            $("._swftarget").removeClass("_ltebtnHover");
        });
        
        io360.publisher.select('link');
    },
    
    initUpload : function() {
        io360.xdswf.transport.initUpload("*.jpeg;*.jpg;*.png;*.gif", 
                                        "io360.publisher.attachment.onSelect", 
                                        "io360.publisher.attachment.onProgress",
                                        "io360.publisher.attachment.onComplete");
    },
    
    select : function(type) {
        io360.publisher.type = type;
        $("._publishSelected").removeClass("_publishSelected");
        $("._publish_" + type).addClass('_publishSelected');
        if (type == 'text') {
            $("#_aux").slideUp(75);
        } else {
            if (type == 'link') {
                $("#_aux div._attachcontainer").hide();
                $("#_aux div._attachwrapper").hide();
                $("#_aux div._inputwrapper").show();
                $("#_aux div._inputwrapper input").attr("default", "Enter a link, attach a comment, and share with your friends").val("Enter a link, attach a comment, and share with your friends");
            } else {
                $("#_aux div._attachcontainer").show();
                $("#_aux div._attachwrapper").show();
                $("#_aux div._inputwrapper").hide();
                $("#_aux div._attachwrapper a i").html("Attach and share an image");
            }
            
            $("#_aux").slideDown(75);
        }
    },
    
    info : function() {
        io360.draw.alert("Hello", "I really am that sexy");
    },

    post : function() {
        var param = $("input.aux").val();
        
        var publisher = $("#_publisher form");        
        var name = publisher.find("div._inputs div._inputwrapper input.name").val();
        var email = publisher.find("div._inputs div._inputwrapper input.email").val();
        var body = publisher.find("div._textwrapper textarea.body").val();
        
        if (io360.publisher.type == 'link' && (param == "" || param == $("input.aux").attr("default"))) {
            io360.draw.alert("Error", "Enter a url first!");
            return;
        }
        
        if (body == "" || body == publisher.find("div._textwrapper textarea.body").attr("default")) {
            io360.draw.alert("Error", "Enter a comment first!");
            return;
        }
        
        if (name == "" || name == publisher.find("div._inputs div._inputwrapper input.name").attr("default")) {
            name = "anonymous";
        }

        if (!io360.util.formsEnabled) return;
        io360.util.disableForms();

        io360.crossDomain.request(
                "http://service.360.io/crossdomain/bridge.html",
                "http://service.360.io/create/head", 
                {
                    type : 'link',
                    name : name,
                    email : email,
                    type : io360.publisher.type,
                    param : io360.publisher.type == 'link' ? $("input.aux").val() : '',
                    body : body,
                    attached : io360.publisher.type == 'pic' ? io360.xdswf.uneval(io360.publisher.attachment.values) : ''
                },
                function(data) {
                    io360.logging.log("io360.landing.submit", data);
                    
                    data = eval("(" + data + ");");

                    io360.draw.alert("Created Post", "<a href='http://360.io/" + data +"'>http://360.io/" + data +"</a>");
                    io360.show.display("Created your post at: <a href='http://360.io/" + data +"'>http://360.io/" + data +"</a><a href='javascript:io360.twitter.tweet(\"http://360.io/" + data + "\");'>Click here to post to twitter.</a>");

                    $("input.aux").val($("input.aux").attr("default"));
                    for (var a in io360.publisher.attachment.values) {
                        delete io360.publisher.attachment.values[a];
                        $("#_attach_" + a).remove();
                    }

                    io360.util.enableForms();
                });        
    },
    
    auxAttachment : function() {
    }
}